
#include <unordered_map>
#include <unordered_set>
#include <algorithm>

#include "s2/s2boolean_operation.h"
#include "s2/s2closest_edge_query.h"
#include "s2/s2furthest_edge_query.h"
#include "s2/s2shape_index_region.h"
#include "s2/s2shape_index_buffered_region.h"

#include "geography-operator.h"
#include "s2-options.h"

#include <Rcpp.h>
using namespace Rcpp;


template<class VectorType, class ScalarType>
class IndexedBinaryGeographyOperator: public UnaryGeographyOperator<VectorType, ScalarType> {
public:
  std::unique_ptr<s2geography::GeographyIndex> geog2_index;
  std::unique_ptr<s2geography::GeographyIndex::Iterator> iterator;

  // max_edges_per_cell should be between 10 and 50, with lower numbers
  // leading to more memory usage (but potentially faster query times). Benchmarking
  // with binary prediates seems to indicate that values on the high end
  // of the spectrum do a reasonable job of efficient preselection, and that
  // decreasing this value does little to increase performance.

  IndexedBinaryGeographyOperator(int maxEdgesPerCell = 50) {
    MutableS2ShapeIndex::Options index_options;
    index_options.set_max_edges_per_cell(maxEdgesPerCell);
    geog2_index = absl::make_unique<s2geography::GeographyIndex>(index_options);
  }

  virtual void buildIndex(List geog2) {
    for (R_xlen_t j = 0; j < geog2.size(); j++) {
      checkUserInterrupt();
      SEXP item2 = geog2[j];

      // build index and store index IDs so that shapeIds can be
      // mapped back to the geog index
      if (item2 == R_NilValue) {
        Rcpp::stop("Missing `y` not allowed in binary indexed operators()");
      } else {
        Rcpp::XPtr<RGeography> feature2(item2);
        geog2_index->Add(feature2->Geog(), j);
      }
    }

    iterator = absl::make_unique<s2geography::GeographyIndex::Iterator>(geog2_index.get());
  }
};

// -------- closest/farthest feature ----------

// [[Rcpp::export]]
IntegerVector cpp_s2_closest_feature(List geog1, List geog2) {

  class Op: public IndexedBinaryGeographyOperator<IntegerVector, int> {
  public:
    int processFeature(Rcpp::XPtr<RGeography> feature, R_xlen_t i) {
      S2ClosestEdgeQuery query(&geog2_index->ShapeIndex());
      S2ClosestEdgeQuery::ShapeIndexTarget target(&feature->Index().ShapeIndex());
      const auto& result = query.FindClosestEdge(&target);
      if (result.is_empty()) {
        return NA_INTEGER;
      } else {
        // convert to R index (+1)
        return geog2_index->value(result.shape_id()) + 1;
      }
    }
  };

  Op op;
  op.buildIndex(geog2);
  return op.processVector(geog1);
}

// [[Rcpp::export]]
IntegerVector cpp_s2_farthest_feature(List geog1, List geog2) {

  class Op: public IndexedBinaryGeographyOperator<IntegerVector, int> {
  public:
    int processFeature(Rcpp::XPtr<RGeography> feature, R_xlen_t i) {
      S2FurthestEdgeQuery query(&geog2_index->ShapeIndex());
      S2FurthestEdgeQuery::ShapeIndexTarget target(&feature->Index().ShapeIndex());
      const auto& result = query.FindFurthestEdge(&target);
      if (result.is_empty()) {
        return NA_INTEGER;
      } else {
        // convert to R index (+1)
        return geog2_index->value(result.shape_id()) + 1;
      }
    }
  };

  Op op;
  op.buildIndex(geog2);
  return op.processVector(geog1);
}

// [[Rcpp::export]]
List cpp_s2_closest_edges(List geog1, List geog2, int n, double min_distance,
                          double max_distance) {

  class Op: public IndexedBinaryGeographyOperator<List, IntegerVector> {
  public:
    IntegerVector processFeature(Rcpp::XPtr<RGeography> feature, R_xlen_t i) {
      S2ClosestEdgeQuery query(&geog2_index->ShapeIndex());
      query.mutable_options()->set_max_results(n);
      query.mutable_options()->set_max_distance(S1ChordAngle::Radians(max_distance));
      S2ClosestEdgeQuery::ShapeIndexTarget target(&feature->Index().ShapeIndex());
      const auto& result = query.FindClosestEdges(&target);

      // this code searches edges, which may come from the same feature
      std::unordered_set<int> features;
      for (S2ClosestEdgeQuery::Result res : result) {
        if (res.distance().radians() > this->min_distance) {
          features.insert(geog2_index->value(res.shape_id()) + 1);
        }
      }

      return IntegerVector(features.begin(), features.end());
    }

    int n;
    double min_distance;
    double max_distance;
  };

  Op op;
  op.n = n;
  op.min_distance = min_distance;
  op.max_distance = max_distance;
  op.buildIndex(geog2);
  return op.processVector(geog1);
}

// ----------- indexed binary predicate operators -----------

class IndexedMatrixPredicateOperator: public IndexedBinaryGeographyOperator<List, IntegerVector> {
public:
  // a max_cells value of 8 was suggested in the S2RegionCoverer docs as a
  // reasonable approximation of a geometry, although benchmarking seems to indicate that
  // increasing this number above 4 actually decreasses performance (using a value
  // of 1 dramatically decreases performance)
  IndexedMatrixPredicateOperator(List s2options, int maxFeatureCells = 4,
                                 int maxEdgesPerCell = 50):
    IndexedBinaryGeographyOperator<List, IntegerVector>(maxEdgesPerCell),
    maxFeatureCells(maxFeatureCells) {
    GeographyOperationOptions options(s2options);
    this->options = options.booleanOperationOptions();
    this->coverer.mutable_options()->set_max_cells(maxFeatureCells);
  }

  void buildIndex(List geog2) {
    this->geog2  = geog2;
    IndexedBinaryGeographyOperator<List, IntegerVector>::buildIndex(geog2);
  }

  IntegerVector processFeature(Rcpp::XPtr<RGeography> feature, R_xlen_t i) {
    coverer.GetCovering(*feature->Geog().Region(), &cell_ids);
    indices_unsorted.clear();
    iterator->Query(cell_ids, &indices_unsorted);

    // loop through features from geog2 that might intersect feature
    // and build a list of indices that actually intersect (based on
    // this->actuallyIntersects(), which might perform alternative
    // comparisons)
    indices.clear();
    for (int j: indices_unsorted) {
      SEXP item = this->geog2[j];
      XPtr<RGeography> feature2(item);

      if (this->actuallyIntersects(feature->Index(), feature2->Index(), i, j)) {
        // convert to R index here + 1
        indices.push_back(j + 1);
      }
    }

    // return sorted integer vector
    std::sort(indices.begin(), indices.end());
    return Rcpp::IntegerVector(indices.begin(), indices.end());
  };

  virtual bool actuallyIntersects(const s2geography::ShapeIndexGeography& index1,
                                  const s2geography::ShapeIndexGeography& index2,
                                  R_xlen_t i, R_xlen_t j) = 0;

  protected:
    List geog2;
    S2BooleanOperation::Options options;
    int maxFeatureCells;
    S2RegionCoverer coverer;
    std::vector<S2CellId> cell_ids;
    std::unordered_set<int> indices_unsorted;
    std::vector<int> indices;
};

// [[Rcpp::export]]
List cpp_s2_may_intersect_matrix(List geog1, List geog2,
                                 int maxEdgesPerCell, int maxFeatureCells, List s2options) {
  class Op: public IndexedMatrixPredicateOperator {
  public:
    Op(List s2options, int maxFeatureCells, int maxEdgesPerCell):
      IndexedMatrixPredicateOperator(s2options, maxFeatureCells, maxEdgesPerCell) {}

    bool actuallyIntersects(const s2geography::ShapeIndexGeography& index1,
                                  const s2geography::ShapeIndexGeography& index2,
                                  R_xlen_t i, R_xlen_t j) {
      return true;
    };
  };

  Op op(s2options, maxFeatureCells, maxEdgesPerCell);
  op.buildIndex(geog2);
  return op.processVector(geog1);
}

// [[Rcpp::export]]
List cpp_s2_contains_matrix(List geog1, List geog2, List s2options) {
  class Op: public IndexedMatrixPredicateOperator {
  public:
    Op(List s2options): IndexedMatrixPredicateOperator(s2options) {}
    bool actuallyIntersects(const s2geography::ShapeIndexGeography& index1,
                                  const s2geography::ShapeIndexGeography& index2,
                                  R_xlen_t i, R_xlen_t j) {
      return s2geography::s2_contains(index1, index2, this->options);
    };
  };

  Op op(s2options);
  op.buildIndex(geog2);
  return op.processVector(geog1);
}

// [[Rcpp::export]]
List cpp_s2_within_matrix(List geog1, List geog2, List s2options) {
  class Op: public IndexedMatrixPredicateOperator {
  public:
    Op(List s2options): IndexedMatrixPredicateOperator(s2options) {}
    bool actuallyIntersects(const s2geography::ShapeIndexGeography& index1,
                                  const s2geography::ShapeIndexGeography& index2,
                                  R_xlen_t i, R_xlen_t j) {
      // note reversed index2, index1
      return s2geography::s2_contains(index2, index1, this->options);
    };
  };

  Op op(s2options);
  op.buildIndex(geog2);
  return op.processVector(geog1);
}

// [[Rcpp::export]]
List cpp_s2_intersects_matrix(List geog1, List geog2, List s2options) {
  class Op: public IndexedMatrixPredicateOperator {
  public:
    Op(List s2options): IndexedMatrixPredicateOperator(s2options) {}
    bool actuallyIntersects(const s2geography::ShapeIndexGeography& index1,
                                  const s2geography::ShapeIndexGeography& index2,
                                  R_xlen_t i, R_xlen_t j) {
      return s2geography::s2_intersects(index1, index2, this->options);
    };
  };

  Op op(s2options);
  op.buildIndex(geog2);
  return op.processVector(geog1);
}

// [[Rcpp::export]]
List cpp_s2_equals_matrix(List geog1, List geog2, List s2options) {
  class Op: public IndexedMatrixPredicateOperator {
  public:
    Op(List s2options): IndexedMatrixPredicateOperator(s2options) {}
    bool actuallyIntersects(const s2geography::ShapeIndexGeography& index1,
                                  const s2geography::ShapeIndexGeography& index2,
                                  R_xlen_t i, R_xlen_t j) {
      return s2geography::s2_equals(index1, index2, this->options);
    };
  };

  Op op(s2options);
  op.buildIndex(geog2);
  return op.processVector(geog1);
}

// [[Rcpp::export]]
List cpp_s2_touches_matrix(List geog1, List geog2, List s2options) {
  class Op: public IndexedMatrixPredicateOperator {
  public:
    Op(List s2options): IndexedMatrixPredicateOperator(s2options) {
      this->closedOptions = this->options;
      this->closedOptions.set_polygon_model(S2BooleanOperation::PolygonModel::CLOSED);
      this->closedOptions.set_polyline_model(S2BooleanOperation::PolylineModel::CLOSED);

      this->openOptions = this->options;
      this->openOptions.set_polygon_model(S2BooleanOperation::PolygonModel::OPEN);
      this->openOptions.set_polyline_model(S2BooleanOperation::PolylineModel::OPEN);
    }

    bool actuallyIntersects(const s2geography::ShapeIndexGeography& index1,
                                  const s2geography::ShapeIndexGeography& index2,
                                  R_xlen_t i, R_xlen_t j) {
      return s2geography::s2_intersects(index1, index2, this->closedOptions) &&
        !s2geography::s2_intersects(index1, index2, this->openOptions);
    };

  private:
    S2BooleanOperation::Options closedOptions;
    S2BooleanOperation::Options openOptions;
  };

  Op op(s2options);
  op.buildIndex(geog2);
  return op.processVector(geog1);
}


// ----------- brute force binary predicate operators ------------------

class BruteForceMatrixPredicateOperator {
public:
  std::vector<S2ShapeIndex*> geog2Indices;
  S2BooleanOperation::Options options;

  BruteForceMatrixPredicateOperator() {}

  BruteForceMatrixPredicateOperator(Rcpp::List s2options) {
    GeographyOperationOptions options(s2options);
    this->options = options.booleanOperationOptions();
  }

  List processVector(Rcpp::List geog1, Rcpp::List geog2) {
    List output(geog1.size());

    // using instead of IntegerVector because
    // std::vector is much faster with repeated calls to .push_back()
    std::vector<int> trueIndices;

    for (R_xlen_t i = 0; i < geog1.size(); i++) {
      trueIndices.clear();

      SEXP item1 = geog1[i];
      if (item1 ==  R_NilValue) {
        output[i] = R_NilValue;
      } else {
        Rcpp::XPtr<RGeography> feature1(item1);

        for (size_t j = 0; j < geog2.size(); j++) {
          checkUserInterrupt();
          SEXP item2 = geog2[j];
          if (item2 == R_NilValue) {
            stop("Missing `y` not allowed in binary index operations");
          }

          XPtr<RGeography> feature2(item2);

          bool result = this->processFeature(feature1, feature2, i, j);
          if (result) {
            // convert to R index here (+1)
            trueIndices.push_back(j + 1);
          }
        }

        IntegerVector itemOut(trueIndices.size());
        for (size_t k = 0; k < trueIndices.size(); k++) {
          itemOut[k] = trueIndices[k];
        }
        output[i] = itemOut;
      }
    }

    return output;
  }

  virtual bool processFeature(XPtr<RGeography> feature1, XPtr<RGeography> feature2,
                              R_xlen_t i, R_xlen_t j) = 0;
};

// [[Rcpp::export]]
List cpp_s2_dwithin_matrix(List geog1, List geog2, double distance) {
  class Op: public IndexedBinaryGeographyOperator<List, IntegerVector> {
  public:
    List geog2;
    S2RegionCoverer coverer;
    std::vector<S2CellId> cell_ids;
    std::unordered_set<int> indices_unsorted;
    std::vector<int> indices;
    S1ChordAngle distance;

    IntegerVector processFeature(Rcpp::XPtr<RGeography> feature1, R_xlen_t i) {
      S2ShapeIndexBufferedRegion buffered(
        &feature1->Index().ShapeIndex(),
        this->distance
      );
      coverer.GetCovering(buffered, &cell_ids);

      indices_unsorted.clear();
      iterator->Query(cell_ids, &indices_unsorted);

      S2ClosestEdgeQuery query(&feature1->Index().ShapeIndex());

      indices.clear();

      for (int j: indices_unsorted) {
        SEXP item = this->geog2[j];
        XPtr<RGeography> feature2(item);

        S2ClosestEdgeQuery::ShapeIndexTarget target(&feature2->Index().ShapeIndex());
        if (query.IsDistanceLessOrEqual(&target, this->distance)) {
          indices.push_back(j + 1);
        }
      }

      // return sorted integer vector
      std::sort(indices.begin(), indices.end());
      return Rcpp::IntegerVector(indices.begin(), indices.end());
    }
  };

  Op op;
  op.geog2 = geog2;
  op.distance = S1ChordAngle::Radians(distance);
  op.buildIndex(geog2);
  return op.processVector(geog1);
}

// ----------- distance matrix operators -------------------

template<class MatrixType, class ScalarType>
class MatrixGeographyOperator {
public:
  MatrixType processVector(Rcpp::List geog1, Rcpp::List geog2) {

    MatrixType output(geog1.size(), geog2.size());

    SEXP item1;
    SEXP item2;

    for (R_xlen_t i = 0; i < geog1.size(); i++) {
      item1 = geog1[i];
      if (item1 ==  R_NilValue) {
        for (R_xlen_t j = 0; j < geog2.size(); j++) {
          output(i, j) = MatrixType::get_na();
        }
      } else {
        Rcpp::XPtr<RGeography> feature1(item1);

        for (R_xlen_t j = 0; j < geog2.size(); j++) {
          checkUserInterrupt();
          item2 = geog2[j];

          if (item2 == R_NilValue) {
            output(i, j) = MatrixType::get_na();
          } else {
            Rcpp::XPtr<RGeography> feature2(item2);
            output(i, j) = this->processFeature(feature1, feature2, i, j);
          }
        }
      }
    }

    return output;
  }

  virtual ScalarType processFeature(Rcpp::XPtr<RGeography> feature1,
                                    Rcpp::XPtr<RGeography> feature2,
                                    R_xlen_t i, R_xlen_t j) = 0;
};

// [[Rcpp::export]]
NumericMatrix cpp_s2_distance_matrix(List geog1, List geog2) {
  class Op: public MatrixGeographyOperator<NumericMatrix, double> {

    double processFeature(XPtr<RGeography> feature1, XPtr<RGeography> feature2,
                          R_xlen_t i, R_xlen_t j) {
      S2ClosestEdgeQuery query(&feature1->Index().ShapeIndex());
      S2ClosestEdgeQuery::ShapeIndexTarget target(&feature2->Index().ShapeIndex());
      const auto& result = query.FindClosestEdge(&target);

      S1ChordAngle angle = result.distance();
      double distance = angle.ToAngle().radians();

      if (distance == R_PosInf) {
        return NA_REAL;
      } else {
        return distance;
      }
    }
  };

  Op op;
  return op.processVector(geog1, geog2);
}

// [[Rcpp::export]]
NumericMatrix cpp_s2_max_distance_matrix(List geog1, List geog2) {
  class Op: public MatrixGeographyOperator<NumericMatrix, double> {

    double processFeature(XPtr<RGeography> feature1, XPtr<RGeography> feature2,
                          R_xlen_t i, R_xlen_t j) {
      S2FurthestEdgeQuery query(&feature1->Index().ShapeIndex());
      S2FurthestEdgeQuery::ShapeIndexTarget target(&feature2->Index().ShapeIndex());
      const auto& result = query.FindFurthestEdge(&target);

      S1ChordAngle angle = result.distance();
      double distance = angle.ToAngle().radians();

      // returns -1 if one of the indexes is empty
      // NA is more consistent with the BigQuery
      // function, and makes way more sense
      if (distance < 0) {
        return NA_REAL;
      } else {
        return distance;
      }
    }
  };

  Op op;
  return op.processVector(geog1, geog2);
}


// ----------- brute force binary predicate operators (for testing) ------------------

// [[Rcpp::export]]
List cpp_s2_contains_matrix_brute_force(List geog1, List geog2, List s2options) {
  class Op: public BruteForceMatrixPredicateOperator {
  public:
    Op(List s2options): BruteForceMatrixPredicateOperator(s2options) {}
    bool processFeature(XPtr<RGeography> feature1, XPtr<RGeography> feature2,
                        R_xlen_t i, R_xlen_t j) {
      return s2geography::s2_contains(feature1->Index(), feature2->Index(), options);
    };
  };

  Op op(s2options);
  return op.processVector(geog1, geog2);
}

// [[Rcpp::export]]
List cpp_s2_within_matrix_brute_force(List geog1, List geog2, List s2options) {
  class Op: public BruteForceMatrixPredicateOperator {
  public:
    Op(List s2options): BruteForceMatrixPredicateOperator(s2options) {}
    bool processFeature(XPtr<RGeography> feature1, XPtr<RGeography> feature2,
                        R_xlen_t i, R_xlen_t j) {
      // note reversed index2, index1
      return s2geography::s2_contains(feature2->Index(), feature1->Index(), options);
    };
  };

  Op op(s2options);
  return op.processVector(geog1, geog2);
}

// [[Rcpp::export]]
List cpp_s2_intersects_matrix_brute_force(List geog1, List geog2, List s2options) {
  class Op: public BruteForceMatrixPredicateOperator {
  public:
    Op(List s2options): BruteForceMatrixPredicateOperator(s2options) {}
    bool processFeature(XPtr<RGeography> feature1, XPtr<RGeography> feature2,
                        R_xlen_t i, R_xlen_t j) {
      return s2geography::s2_intersects(feature1->Index(), feature2->Index(), options);
    }
  };

  Op op(s2options);
  return op.processVector(geog1, geog2);
}

// [[Rcpp::export]]
List cpp_s2_disjoint_matrix_brute_force(List geog1, List geog2, List s2options) {
  class Op: public BruteForceMatrixPredicateOperator {
  public:
    Op(List s2options): BruteForceMatrixPredicateOperator(s2options) {}
    bool processFeature(XPtr<RGeography> feature1, XPtr<RGeography> feature2,
                        R_xlen_t i, R_xlen_t j) {
      return !s2geography::s2_intersects(feature1->Index(), feature2->Index(), options);
    }
  };

  Op op(s2options);
  return op.processVector(geog1, geog2);
}

// [[Rcpp::export]]
List cpp_s2_equals_matrix_brute_force(List geog1, List geog2, List s2options) {
  class Op: public BruteForceMatrixPredicateOperator {
  public:
    Op(List s2options): BruteForceMatrixPredicateOperator(s2options) {}
    bool processFeature(XPtr<RGeography> feature1, XPtr<RGeography> feature2,
                        R_xlen_t i, R_xlen_t j) {
      return s2geography::s2_equals(feature1->Index(), feature2->Index(), options);
    }
  };

  Op op(s2options);
  return op.processVector(geog1, geog2);
}

// [[Rcpp::export]]
List cpp_s2_dwithin_matrix_brute_force(List geog1, List geog2, double distance) {
  class Op: public BruteForceMatrixPredicateOperator {
  public:
    double distance;
    Op(double distance): distance(distance) {}
    bool processFeature(XPtr<RGeography> feature1, XPtr<RGeography> feature2,
                        R_xlen_t i, R_xlen_t j) {
      S2ClosestEdgeQuery query(&feature2->Index().ShapeIndex());
      S2ClosestEdgeQuery::ShapeIndexTarget target(&feature1->Index().ShapeIndex());
      return query.IsDistanceLessOrEqual(&target, S1ChordAngle::Radians(this->distance));
    };
  };

  Op op(distance);
  return op.processVector(geog1, geog2);
}
