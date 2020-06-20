
#include "s2/s2boolean_operation.h"
#include "s2/s2closest_edge_query.h"
#include "s2/s2furthest_edge_query.h"

#include "geography-operator.h"
#include "s2-options.h"

#include <Rcpp.h>
using namespace Rcpp;



template<class VectorType, class ScalarType>
class IndexedBinaryGeographyOperator: public UnaryGeographyOperator<VectorType, ScalarType> {
public:
  MutableS2ShapeIndex geog2Index;
  std::unordered_map<int, R_xlen_t> geog2IndexSource;

  void buildIndex(List geog2) {
    SEXP item2;
    std::vector<int> shapeIds;

    for (R_xlen_t j = 0; j < geog2.size(); j++) {
      item2 = geog2[j];

      // build index and store index IDs so that shapeIds can be
      // mapped back to the geog2 index
      if (item2 == R_NilValue) {
        Rcpp::stop("Missing `y` not allowed in binary indexed operators()");
      } else {
        Rcpp::XPtr<Geography> feature2(item2);
        shapeIds = feature2->BuildShapeIndex(&(this->geog2Index));
        for (size_t k = 0; k < shapeIds.size(); k ++) {
          geog2IndexSource[shapeIds[k]] = j;
        }
      }
    }
  }
};

// -------- closest/farthest feature ----------

// [[Rcpp::export]]
IntegerVector cpp_s2_closest_feature(List geog1, List geog2) {

  class Op: public IndexedBinaryGeographyOperator<IntegerVector, int> {
  public:
    int processFeature(Rcpp::XPtr<Geography> feature, R_xlen_t i) {
      S2ClosestEdgeQuery query(&(this->geog2Index));
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
      S2FurthestEdgeQuery query(&(this->geog2Index));
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

// ----------- binary predicate operators ------------------

class MatrixPredicateOperator {
public:
  std::vector<S2ShapeIndex*> geog2Indices;
  S2BooleanOperation::Options options;

  MatrixPredicateOperator() {}

  MatrixPredicateOperator(Rcpp::List s2options) {
    GeographyOperationOptions options(s2options);
    this->options = options.booleanOperationOptions();
  }

  void buildIndex(List geog2) {
    SEXP item2;
    this->geog2Indices = std::vector<S2ShapeIndex*>(geog2.size());

    for (R_xlen_t j = 0; j < geog2.size(); j++) {
      item2 = geog2[j];

      if (item2 == R_NilValue) {
        Rcpp::stop("Missing `y` not allowed in binary indexed operators()");
      } else {
        Rcpp::XPtr<Geography> feature2(item2);
        geog2Indices[j] = feature2->ShapeIndex();
      }
    }
  }

  List processVector(Rcpp::List geog1) {
    List output(geog1.size());
    SEXP item1;
    // using instead of IntegerVector because
    // std::vector is much faster with repeated calls to
    // push_back()
    // alternatively, could allocate one logical vector and
    // use Rcpp::which() or similar
    std::vector<int> trueIndices;

    for (R_xlen_t i = 0; i < geog1.size(); i++) {
      trueIndices.clear();

      item1 = geog1[i];
      if (item1 ==  R_NilValue) {
        output[i] = R_NilValue;
      } else {
        Rcpp::XPtr<Geography> feature1(item1);

        for (size_t j = 0; j < this->geog2Indices.size(); j++) {
          bool result = this->processFeature(
            feature1->ShapeIndex(),
            this->geog2Indices[j],
            i, j
          );

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

  virtual bool processFeature(S2ShapeIndex* index1, S2ShapeIndex* index2,
                              R_xlen_t i, R_xlen_t j) = 0;
};

// [[Rcpp::export]]
List cpp_s2_contains_matrix(List geog1, List geog2, List s2options) {
  class Op: public MatrixPredicateOperator {
  public:
    Op(List s2options): MatrixPredicateOperator(s2options) {}
    bool processFeature(S2ShapeIndex* index1, S2ShapeIndex* index2, R_xlen_t i, R_xlen_t j) {
      return S2BooleanOperation::Contains(*index1, *index2, this->options);
    };
  };

  Op op(s2options);
  op.buildIndex(geog2);
  return op.processVector(geog1);
}

// [[Rcpp::export]]
List cpp_s2_within_matrix(List geog1, List geog2, List s2options) {
  class Op: public MatrixPredicateOperator {
  public:
    Op(List s2options): MatrixPredicateOperator(s2options) {}
    bool processFeature(S2ShapeIndex* index1, S2ShapeIndex* index2, R_xlen_t i, R_xlen_t j) {
      return S2BooleanOperation::Contains(*index2, *index1, this->options);
    };
  };

  Op op(s2options);
  op.buildIndex(geog2);
  return op.processVector(geog1);
}

// [[Rcpp::export]]
List cpp_s2_intersects_matrix(List geog1, List geog2, List s2options) {
  class Op: public MatrixPredicateOperator {
  public:
    Op(List s2options): MatrixPredicateOperator(s2options) {}
    bool processFeature(S2ShapeIndex* index1, S2ShapeIndex* index2, R_xlen_t i, R_xlen_t j) {
      return S2BooleanOperation::Intersects(*index1, *index2, this->options);
    };
  };

  Op op(s2options);
  op.buildIndex(geog2);
  return op.processVector(geog1);
}

// [[Rcpp::export]]
List cpp_s2_disjoint_matrix(List geog1, List geog2, List s2options) {
  class Op: public MatrixPredicateOperator {
  public:
    Op(List s2options): MatrixPredicateOperator(s2options) {}
    bool processFeature(S2ShapeIndex* index1, S2ShapeIndex* index2, R_xlen_t i, R_xlen_t j) {
      return !S2BooleanOperation::Intersects(*index1, *index2, this->options);
    };
  };

  Op op(s2options);
  op.buildIndex(geog2);
  return op.processVector(geog1);
}

// [[Rcpp::export]]
List cpp_s2_equals_matrix(List geog1, List geog2, List s2options) {
  class Op: public MatrixPredicateOperator {
  public:
    Op(List s2options): MatrixPredicateOperator(s2options) {}
    bool processFeature(S2ShapeIndex* index1, S2ShapeIndex* index2, R_xlen_t i, R_xlen_t j) {
      return S2BooleanOperation::Equals(*index1, *index2, this->options);
    };
  };

  Op op(s2options);
  op.buildIndex(geog2);
  return op.processVector(geog1);
}

// [[Rcpp::export]]
List cpp_s2_dwithin_matrix(List geog1, List geog2, double distance) {
  class Op: public MatrixPredicateOperator {
  public:
    double distance;
    Op(double distance): distance(distance) {}
    bool processFeature(S2ShapeIndex* index1, S2ShapeIndex* index2, R_xlen_t i, R_xlen_t j) {
      S2ClosestEdgeQuery query(index2);
      S2ClosestEdgeQuery::ShapeIndexTarget target(index1);
      return query.IsDistanceLessOrEqual(&target, S1ChordAngle::Radians(this->distance));
    };
  };

  Op op(distance);
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
        Rcpp::XPtr<Geography> feature1(item1);

        for (R_xlen_t j = 0; j < geog2.size(); j++) {
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

    double processFeature(XPtr<Geography> feature1,
                          XPtr<Geography> feature2,
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

    double processFeature(XPtr<Geography> feature1,
                          XPtr<Geography> feature2,
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
