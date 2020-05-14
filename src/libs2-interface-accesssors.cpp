
#include "libs2-s2geography.h"
#include "s2/s2closest_edge_query.h"
#include <Rcpp.h>
using namespace Rcpp;

template<class VectorType, class ScalarType>
class S2Accessor {
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
  class S2Op: public S2Accessor<LogicalVector, int> {
    int processFeature(XPtr<LibS2Geography> feature) {
      return feature->IsCollection();
    }
  };

  S2Op op;
  return op.processVector(geog);
}

// [[Rcpp::export]]
IntegerVector libs2_cpp_s2_dimension(List geog) {
  class S2Op: public S2Accessor<IntegerVector, int> {
    int processFeature(XPtr<LibS2Geography> feature) {
      return feature->Dimension();
    }
  };

  S2Op op;
  return op.processVector(geog);
}

// [[Rcpp::export]]
IntegerVector libs2_cpp_s2_numpoints(List geog) {
  class S2Op: public S2Accessor<IntegerVector, int> {
    int processFeature(XPtr<LibS2Geography> feature) {
      return feature->NumPoints();
    }
  };

  S2Op op;
  return op.processVector(geog);
}

// [[Rcpp::export]]
NumericVector libs2_cpp_s2_area(List geog) {
  class S2Op: public S2Accessor<NumericVector, double> {
    double processFeature(XPtr<LibS2Geography> feature) {
      return feature->Area();
    }
  };

  S2Op op;
  return op.processVector(geog);
}

// [[Rcpp::export]]
NumericVector libs2_cpp_s2_length(List geog) {
  class S2Op: public S2Accessor<NumericVector, double> {
    double processFeature(XPtr<LibS2Geography> feature) {
      return feature->Length();
    }
  };

  S2Op op;
  return op.processVector(geog);
}

// [[Rcpp::export]]
NumericVector libs2_cpp_s2_perimeter(List geog) {
  class S2Op: public S2Accessor<NumericVector, double> {
    double processFeature(XPtr<LibS2Geography> feature) {
      return feature->Perimeter();
    }
  };

  S2Op op;
  return op.processVector(geog);
}

// [[Rcpp::export]]
NumericVector libs2_cpp_s2_x(List geog) {
  class S2Op: public S2Accessor<NumericVector, double> {
    double processFeature(XPtr<LibS2Geography> feature) {
      return feature->X();
    }
  };

  S2Op op;
  return op.processVector(geog);
}

// [[Rcpp::export]]
NumericVector libs2_cpp_s2_y(List geog) {
  class S2Op: public S2Accessor<NumericVector, double> {
    double processFeature(XPtr<LibS2Geography> feature) {
      return feature->Y();
    }
  };

  S2Op op;
  return op.processVector(geog);
}

// [[Rcpp::export]]
NumericVector libs2_cpp_s2_distance(List geog1, List geog2) {
  NumericVector output(geog1.size());
  if (geog2.size() != geog1.size()) {
    stop("Incompatible lengths");
  }

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
      S2ClosestEdgeQuery query(feature1->ShapeIndex());
      S2ClosestEdgeQuery::ShapeIndexTarget target(feature2->ShapeIndex());

      const auto& result = query.FindClosestEdge(&target);
      S1ChordAngle angle = result.distance();
      output[i] = angle.ToAngle().radians();
    }
  }

  return output;
}

