
#include "libs2-s2geography.h"
#include "s2/s2closest_edge_query.h"
#include <Rcpp.h>
using namespace Rcpp;

template<class VectorType, class ScalarType>
class LibS2UnaryOperator {
public:
  VectorType processVector(List geog) {
    VectorType output(geog.size());

    SEXP item;
    for (R_xlen_t i = 0; i < geog.size(); i++) {
      item = geog[i];
      if (item == R_NilValue) {
        output[i] = VectorType::get_na();
      } else {
        XPtr<LibS2Geography> feature(item);
        output[i] = this->processFeature(feature);
      }
    }

    return output;
  }

  virtual ScalarType processFeature(XPtr<LibS2Geography> feature) = 0;
};

// [[Rcpp::export]]
LogicalVector libs2_cpp_s2_iscollection(List geog) {
  class LibS2Op: public LibS2UnaryOperator<LogicalVector, int> {
    int processFeature(XPtr<LibS2Geography> feature) {
      return feature->IsCollection();
    }
  };

  LibS2Op op;
  return op.processVector(geog);
}

// [[Rcpp::export]]
IntegerVector libs2_cpp_s2_dimension(List geog) {
  class LibS2Op: public LibS2UnaryOperator<IntegerVector, int> {
    int processFeature(XPtr<LibS2Geography> feature) {
      return feature->Dimension();
    }
  };

  LibS2Op op;
  return op.processVector(geog);
}

// [[Rcpp::export]]
IntegerVector libs2_cpp_s2_numpoints(List geog) {
  class LibS2Op: public LibS2UnaryOperator<IntegerVector, int> {
    int processFeature(XPtr<LibS2Geography> feature) {
      return feature->NumPoints();
    }
  };

  LibS2Op op;
  return op.processVector(geog);
}

// [[Rcpp::export]]
NumericVector libs2_cpp_s2_area(List geog) {
  class LibS2Op: public LibS2UnaryOperator<NumericVector, double> {
    double processFeature(XPtr<LibS2Geography> feature) {
      return feature->Area();
    }
  };

  LibS2Op op;
  return op.processVector(geog);
}

// [[Rcpp::export]]
NumericVector libs2_cpp_s2_length(List geog) {
  class LibS2Op: public LibS2UnaryOperator<NumericVector, double> {
    double processFeature(XPtr<LibS2Geography> feature) {
      return feature->Length();
    }
  };

  LibS2Op op;
  return op.processVector(geog);
}

// [[Rcpp::export]]
NumericVector libs2_cpp_s2_perimeter(List geog) {
  class LibS2Op: public LibS2UnaryOperator<NumericVector, double> {
    double processFeature(XPtr<LibS2Geography> feature) {
      return feature->Perimeter();
    }
  };

  LibS2Op op;
  return op.processVector(geog);
}

// [[Rcpp::export]]
NumericVector libs2_cpp_s2_x(List geog) {
  class LibS2Op: public LibS2UnaryOperator<NumericVector, double> {
    double processFeature(XPtr<LibS2Geography> feature) {
      return feature->X();
    }
  };

  LibS2Op op;
  return op.processVector(geog);
}

// [[Rcpp::export]]
NumericVector libs2_cpp_s2_y(List geog) {
  class LibS2Op: public LibS2UnaryOperator<NumericVector, double> {
    double processFeature(XPtr<LibS2Geography> feature) {
      return feature->Y();
    }
  };

  LibS2Op op;
  return op.processVector(geog);
}

template<class VectorType, class ScalarType>
class LibS2BinaryOperator {
public:
  VectorType processVector(List geog1, List geog2) {
    if (geog2.size() != geog1.size()) {
      stop("Incompatible lengths");
    }

    VectorType output(geog1.size());

    SEXP item1;
    SEXP item2;

    for (R_xlen_t i = 0; i < geog1.size(); i++) {
      item1 = geog1[i];
      item2 = geog2[i];
      if (item1 ==  R_NilValue || item2 == R_NilValue) {
        output[i] = NA_REAL;
      } else {
        XPtr<LibS2Geography> feature1(item1);
        XPtr<LibS2Geography> feature2(item2);
        output[i] = processFeature(feature1, feature2);
      }
    }

    return output;
  }

  virtual ScalarType processFeature(XPtr<LibS2Geography> feature1, XPtr<LibS2Geography> feature2) = 0;
};

// [[Rcpp::export]]
NumericVector libs2_cpp_s2_distance(List geog1, List geog2) {
  class LibS2Op: public LibS2BinaryOperator<NumericVector, double> {

    double processFeature(XPtr<LibS2Geography> feature1, XPtr<LibS2Geography> feature2) {
      S2ClosestEdgeQuery query(feature1->ShapeIndex());
      S2ClosestEdgeQuery::ShapeIndexTarget target(feature2->ShapeIndex());

      const auto& result = query.FindClosestEdge(&target);

      S1ChordAngle angle = result.distance();
      return angle.ToAngle().radians();
    }
  };

  LibS2Op op;
  return op.processVector(geog1, geog2);
}
