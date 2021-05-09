
#include <unordered_map>
#include <unordered_set>
#include <algorithm>

#include "s2/s2boolean_operation.h"
#include "s2/s2closest_edge_query.h"
#include "s2/s2furthest_edge_query.h"
#include "s2/s2shape_index_region.h"

#include "geography-operator.h"
#include "s2-options.h"

#include <Rcpp.h>
using namespace Rcpp;

std::unordered_map<int, R_xlen_t> buildSourcedIndex(List geog, MutableS2ShapeIndex* index) {
  std::unordered_map<int, R_xlen_t> indexSource;
  std::vector<int> shapeIds;

  for (R_xlen_t j = 0; j < geog.size(); j++) {
    checkUserInterrupt();
    SEXP item2 = geog[j];

    // build index and store index IDs so that shapeIds can be
    // mapped back to the geog index
    if (item2 == R_NilValue) {
      Rcpp::stop("Missing `y` not allowed in binary indexed operators()");
    } else {
      Rcpp::XPtr<Geography> feature2(item2);
      shapeIds = feature2->BuildShapeIndex(index);
      for (size_t k = 0; k < shapeIds.size(); k ++) {
        indexSource[shapeIds[k]] = j;
      }
    }
  }

  return indexSource;
}

std::unordered_set<R_xlen_t> findPossibleIntersections(const S2Region& region, 
                                                       const MutableS2ShapeIndex* index,
                                                       std::unordered_map<int, R_xlen_t>& source,
                                                       int maxRegionCells) {
  
  std::unordered_set<R_xlen_t> mightIntersectIndices;
  MutableS2ShapeIndex::Iterator indexIterator(index);

  // generate a small covering of the region
  S2RegionCoverer coverer;
  coverer.mutable_options()->set_max_cells(maxRegionCells);
  S2CellUnion covering = coverer.GetCovering(region);

  // iterate over cells in the featureIndex
  for (S2CellId featureCellId: covering) {
    S2ShapeIndex::CellRelation relation = indexIterator.Locate(featureCellId);

    if (relation == S2ShapeIndex::CellRelation::INDEXED) {
      // we're in luck! these indexes have this cell in common
      // add all the features it contains as possible intersectors for featureIndex
      const S2ShapeIndexCell& cell = indexIterator.cell();
      for (int k = 0; k < cell.num_clipped(); k++) {
        int shapeId = cell.clipped(k).shape_id();
        mightIntersectIndices.insert(source[shapeId]);
      }
    
    } else if(relation  == S2ShapeIndex::CellRelation::SUBDIVIDED) {
      // promising! the geog2 index has a child cell of it.id()
      // (at which indexIterator is now positioned)
      // keep iterating until the iterator is done OR we're no longer at a child cell of
      // it.id(). The ordering of the iterator isn't guaranteed anywhere in the documentation;
      // however, this ordering would be consistent with that of a Normalized
      // S2CellUnion.
      while (!indexIterator.done() && featureCellId.contains(indexIterator.id())) {
        // potentially many cells in the indexIterator, so let the user cancel if this is
        // running too long
        checkUserInterrupt();

        // add all the features the child cell contains as possible intersectors for featureIndex
        const S2ShapeIndexCell& cell = indexIterator.cell();
        for (int k = 0; k < cell.num_clipped(); k++) {
          int shapeId = cell.clipped(k).shape_id();
          mightIntersectIndices.insert(source[shapeId]);
        }

        // go to the next cell in the index
        indexIterator.Next();
      }
    }

    // else: relation == S2ShapeIndex::CellRelation::DISJOINT (do nothing)
  }

  return mightIntersectIndices;
}

template<class VectorType, class ScalarType>
class IndexedBinaryGeographyOperator: public UnaryGeographyOperator<VectorType, ScalarType> {
public:
  std::unique_ptr<MutableS2ShapeIndex> geog2Index;
  std::unordered_map<int, R_xlen_t> geog2IndexSource;

  IndexedBinaryGeographyOperator() {
    this->geog2Index = absl::make_unique<MutableS2ShapeIndex>();
  }

  // maxEdgesPerCell should be between 10 and 50, with lower numbers
  // leading to more memory usage (but potentially faster query times). Benchmarking
  // with binary prediates seems to indicate that values on the high end
  // of the spectrum do a reasonable job of efficient preselection, and that
  // decreasing this value does little to increase performance.
  virtual void buildIndex(List geog2, int maxEdgesPerCell = 50) {
    MutableS2ShapeIndex::Options indexOptions;
    indexOptions.set_max_edges_per_cell(maxEdgesPerCell);
    this->geog2Index = absl::make_unique<MutableS2ShapeIndex>(indexOptions);
    this->geog2IndexSource = buildSourcedIndex(geog2, this->geog2Index.get());
  }
};

// -------- closest/farthest feature ----------

// [[Rcpp::export]]
IntegerVector cpp_s2_closest_feature(List geog1, List geog2) {

  class Op: public IndexedBinaryGeographyOperator<IntegerVector, int> {
  public:
    int processFeature(Rcpp::XPtr<Geography> feature, R_xlen_t i) {
      S2ClosestEdgeQuery query(this->geog2Index.get());
      S2ClosestEdgeQuery::ShapeIndexTarget target(feature->ShapeIndex());
      const auto& result = query.FindClosestEdge(&target);
      if (result.is_empty()) {
        return NA_INTEGER;
      } else {
        // convert to R index (+1)
        return this->geog2IndexSource[result.shape_id()] + 1;
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
    int processFeature(Rcpp::XPtr<Geography> feature, R_xlen_t i) {
      S2FurthestEdgeQuery query(this->geog2Index.get());
      S2FurthestEdgeQuery::ShapeIndexTarget target(feature->ShapeIndex());
      const auto& result = query.FindFurthestEdge(&target);
      if (result.is_empty()) {
        return NA_INTEGER;
      } else {
        // convert to R index (+1)
        return this->geog2IndexSource[result.shape_id()] + 1;
      }
    }
  };

  Op op;
  op.buildIndex(geog2);
  return op.processVector(geog1);
}

// [[Rcpp::export]]
List cpp_s2_closest_edges(List geog1, List geog2, int n, double min_distance) {

  class Op: public IndexedBinaryGeographyOperator<List, IntegerVector> {
  public:
    IntegerVector processFeature(Rcpp::XPtr<Geography> feature, R_xlen_t i) {
      S2ClosestEdgeQuery query(this->geog2Index.get());
      query.mutable_options()->set_max_results(n);
      S2ClosestEdgeQuery::ShapeIndexTarget target(feature->ShapeIndex());
      const auto& result = query.FindClosestEdges(&target);

      // this code searches edges, which may come from the same feature
      std::unordered_set<int> features;
      for (S2ClosestEdgeQuery::Result res : result) {
        if (res.distance().radians() > this->min_distance) {
          features.insert(this->geog2IndexSource[res.shape_id()] + 1);
        }
      }

      return IntegerVector(features.begin(), features.end());
    }

    int n;
    double min_distance;
  };

  Op op;
  op.n = n;
  op.min_distance = min_distance;
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
  IndexedMatrixPredicateOperator(List s2options, int maxFeatureCells = 4):
    maxFeatureCells(maxFeatureCells) {
    GeographyOperationOptions options(s2options);
    this->options = options.booleanOperationOptions();
  }

  // See IndexedBinaryGeographyOperator::buildIndex() for why 50 is the default value
  // for maxEdgesPerCell
  void buildIndex(List geog2, int maxEdgesPerCell = 50) {
    this->geog2  = geog2;
    IndexedBinaryGeographyOperator<List, IntegerVector>::buildIndex(geog2, maxEdgesPerCell);
  }

  IntegerVector processFeature(Rcpp::XPtr<Geography> feature, R_xlen_t i) {
    S2ShapeIndex* index1 = feature->ShapeIndex();
    S2ShapeIndexRegion<S2ShapeIndex> region = MakeS2ShapeIndexRegion(index1);

    // build a list of candidate feature indices
    std::unordered_set<R_xlen_t> mightIntersectIndices = findPossibleIntersections(
      region,
      this->geog2Index.get(),
      this->geog2IndexSource,
      this->maxFeatureCells
    );

    // loop through features from geog2 that might intersect feature
    // and build a list of indices that actually intersect (based on
    // this->actuallyIntersects(), which might perform alternative
    // comparisons)
    std::vector<int> actuallyIntersectIndices;
    for (R_xlen_t j: mightIntersectIndices) {
      SEXP item = this->geog2[j];
      XPtr<Geography> feature2(item);
      if (this->actuallyIntersects(index1, feature2->ShapeIndex(), i, j)) {
        // convert to R index here + 1
        actuallyIntersectIndices.push_back(j + 1);
      }
    }

    // return sorted integer vector
    std::sort(actuallyIntersectIndices.begin(), actuallyIntersectIndices.end());
    return Rcpp::IntegerVector(actuallyIntersectIndices.begin(), actuallyIntersectIndices.end());
  };

  virtual bool actuallyIntersects(S2ShapeIndex* index1, S2ShapeIndex* index2, R_xlen_t i, R_xlen_t j) = 0;

  protected:
    List geog2;
    S2BooleanOperation::Options options;
    int maxFeatureCells;
};

// [[Rcpp::export]]
List cpp_s2_may_intersect_matrix(List geog1, List geog2, 
                                 int maxEdgesPerCell, int maxFeatureCells, List s2options) {
  class Op: public IndexedMatrixPredicateOperator {
  public:
    Op(List s2options, int maxFeatureCells): 
      IndexedMatrixPredicateOperator(s2options, maxFeatureCells) {}
    
    bool actuallyIntersects(S2ShapeIndex* index1, S2ShapeIndex* index2, R_xlen_t i, R_xlen_t j) {
      return true;
    };
  };

  Op op(s2options, maxFeatureCells);
  op.buildIndex(geog2, maxEdgesPerCell);
  return op.processVector(geog1);
}

// [[Rcpp::export]]
List cpp_s2_contains_matrix(List geog1, List geog2, List s2options) {
  class Op: public IndexedMatrixPredicateOperator {
  public:
    Op(List s2options): IndexedMatrixPredicateOperator(s2options) {}
    bool actuallyIntersects(S2ShapeIndex* index1, S2ShapeIndex* index2, R_xlen_t i, R_xlen_t j) {
      return S2BooleanOperation::Contains(*index1, *index2, this->options);
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
    bool actuallyIntersects(S2ShapeIndex* index1, S2ShapeIndex* index2, R_xlen_t i, R_xlen_t j) {
      // note reversed index2, index1
      return S2BooleanOperation::Contains(*index2, *index1, this->options);
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
    bool actuallyIntersects(S2ShapeIndex* index1, S2ShapeIndex* index2, R_xlen_t i, R_xlen_t j) {
      return S2BooleanOperation::Intersects(*index1, *index2, this->options);
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
    bool actuallyIntersects(S2ShapeIndex* index1, S2ShapeIndex* index2, R_xlen_t i, R_xlen_t j) {
      return S2BooleanOperation::Equals(*index1, *index2, this->options);
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

    bool actuallyIntersects(S2ShapeIndex* index1, S2ShapeIndex* index2, R_xlen_t i, R_xlen_t j) {
      // efficiently re-uses the index on geog2 and takes advantage of short-circuiting &&
      return S2BooleanOperation::Intersects(*index1, *index2, this->closedOptions) &&
        !S2BooleanOperation::Intersects(*index1, *index2, this->openOptions);
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
        Rcpp::XPtr<Geography> feature1(item1);

        for (size_t j = 0; j < geog2.size(); j++) {
          checkUserInterrupt();
          SEXP item2 = geog2[j];
          if (item2 == R_NilValue) {
            stop("Missing `y` not allowed in binary index operations");
          }

          XPtr<Geography> feature2(item2);

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

  virtual bool processFeature(XPtr<Geography> feature1, XPtr<Geography> feature2,
                              R_xlen_t i, R_xlen_t j) = 0;
};

// [[Rcpp::export]]
List cpp_s2_dwithin_matrix(List geog1, List geog2, double distance) {
  class Op: public BruteForceMatrixPredicateOperator {
  public:
    double distance;
    Op(double distance): distance(distance) {}
    bool processFeature(XPtr<Geography> feature1, XPtr<Geography> feature2,
                        R_xlen_t i, R_xlen_t j) {
      S2ClosestEdgeQuery query(feature2->ShapeIndex());
      S2ClosestEdgeQuery::ShapeIndexTarget target(feature1->ShapeIndex());
      return query.IsDistanceLessOrEqual(&target, S1ChordAngle::Radians(this->distance));
    };
  };

  Op op(distance);
  return op.processVector(geog1, geog2);
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
        Rcpp::XPtr<Geography> feature1(item1);

        for (R_xlen_t j = 0; j < geog2.size(); j++) {
          checkUserInterrupt();
          item2 = geog2[j];

          if (item2 == R_NilValue) {
            output(i, j) = MatrixType::get_na();
          } else {
            Rcpp::XPtr<Geography> feature2(item2);
            output(i, j) = this->processFeature(feature1, feature2, i, j);
          }
        }
      }
    }

    return output;
  }

  virtual ScalarType processFeature(Rcpp::XPtr<Geography> feature1,
                                    Rcpp::XPtr<Geography> feature2,
                                    R_xlen_t i, R_xlen_t j) = 0;
};

// [[Rcpp::export]]
NumericMatrix cpp_s2_distance_matrix(List geog1, List geog2) {
  class Op: public MatrixGeographyOperator<NumericMatrix, double> {

    double processFeature(XPtr<Geography> feature1, XPtr<Geography> feature2,
                          R_xlen_t i, R_xlen_t j) {
      S2ClosestEdgeQuery query(feature1->ShapeIndex());
      S2ClosestEdgeQuery::ShapeIndexTarget target(feature2->ShapeIndex());
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

    double processFeature(XPtr<Geography> feature1, XPtr<Geography> feature2,
                          R_xlen_t i, R_xlen_t j) {
      S2FurthestEdgeQuery query(feature1->ShapeIndex());
      S2FurthestEdgeQuery::ShapeIndexTarget target(feature2->ShapeIndex());
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
    bool processFeature(XPtr<Geography> feature1, XPtr<Geography> feature2,
                        R_xlen_t i, R_xlen_t j) {
      // by default Contains() will return true for Contains(x, EMPTY), which is
      // not true in BigQuery or GEOS
      if (feature2->IsEmpty()) {
        return false;
      } else {
        return S2BooleanOperation::Contains(
          *feature1->ShapeIndex(), 
          *feature2->ShapeIndex(), 
          this->options
        );
      }
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
    bool processFeature(XPtr<Geography> feature1, XPtr<Geography> feature2,
                        R_xlen_t i, R_xlen_t j) {
      // note reversed index2, index1
      
      // by default Contains() will return true for Contains(x, EMPTY), which is
      // not true in BigQuery or GEOS
      if (feature1->IsEmpty()) {
        return false;
      } else {
        return S2BooleanOperation::Contains(
          *feature2->ShapeIndex(),
          *feature1->ShapeIndex(),
          this->options
        );
      }      
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
    bool processFeature(XPtr<Geography> feature1, XPtr<Geography> feature2,
                        R_xlen_t i, R_xlen_t j) {
      return S2BooleanOperation::Intersects(
        *feature1->ShapeIndex(),
        *feature2->ShapeIndex(),
        this->options
      );
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
    bool processFeature(XPtr<Geography> feature1, XPtr<Geography> feature2,
                        R_xlen_t i, R_xlen_t j) {
      return !S2BooleanOperation::Intersects(
        *feature1->ShapeIndex(),
        *feature2->ShapeIndex(),
        this->options
      );
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
    bool processFeature(XPtr<Geography> feature1, XPtr<Geography> feature2,
                        R_xlen_t i, R_xlen_t j) {
      return S2BooleanOperation::Equals(
        *feature1->ShapeIndex(),
        *feature2->ShapeIndex(),
        this->options
      );
    }
  };

  Op op(s2options);
  return op.processVector(geog1, geog2);
}
