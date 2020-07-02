
#include "geography-operator.h"
#include "s2/s2closest_edge_query.h"
#include "s2/s2furthest_edge_query.h"
#include <Rcpp.h>
using namespace Rcpp;

// [[Rcpp::export]]
LogicalVector cpp_s2_is_collection(List geog) {
  class Op: public UnaryGeographyOperator<LogicalVector, int> {
    int processFeature(XPtr<Geography> feature, R_xlen_t i) {
      return feature->IsCollection();
    }
  };

  Op op;
  return op.processVector(geog);
}

// [[Rcpp::export]]
IntegerVector cpp_s2_dimension(List geog) {
  class Op: public UnaryGeographyOperator<IntegerVector, int> {
    int processFeature(XPtr<Geography> feature, R_xlen_t i) {
      return feature->Dimension();
    }
  };

  Op op;
  return op.processVector(geog);
}

// [[Rcpp::export]]
IntegerVector cpp_s2_num_points(List geog) {
  class Op: public UnaryGeographyOperator<IntegerVector, int> {
    int processFeature(XPtr<Geography> feature, R_xlen_t i) {
      return feature->NumPoints();
    }
  };

  Op op;
  return op.processVector(geog);
}

// [[Rcpp::export]]
LogicalVector cpp_s2_is_empty(List geog) {
  class Op: public UnaryGeographyOperator<LogicalVector, int> {
    int processFeature(XPtr<Geography> feature, R_xlen_t i) {
      return feature->IsEmpty();
    }
  };

  Op op;
  return op.processVector(geog);
}

// [[Rcpp::export]]
NumericVector cpp_s2_area(List geog) {
  class Op: public UnaryGeographyOperator<NumericVector, double> {
    double processFeature(XPtr<Geography> feature, R_xlen_t i) {
      return feature->Area();
    }
  };

  Op op;
  return op.processVector(geog);
}

// [[Rcpp::export]]
NumericVector cpp_s2_length(List geog) {
  class Op: public UnaryGeographyOperator<NumericVector, double> {
    double processFeature(XPtr<Geography> feature, R_xlen_t i) {
      return feature->Length();
    }
  };

  Op op;
  return op.processVector(geog);
}

// [[Rcpp::export]]
NumericVector cpp_s2_perimeter(List geog) {
  class Op: public UnaryGeographyOperator<NumericVector, double> {
    double processFeature(XPtr<Geography> feature, R_xlen_t i) {
      return feature->Perimeter();
    }
  };

  Op op;
  return op.processVector(geog);
}

// [[Rcpp::export]]
NumericVector cpp_s2_x(List geog) {
  class Op: public UnaryGeographyOperator<NumericVector, double> {
    double processFeature(XPtr<Geography> feature, R_xlen_t i) {
      return feature->X();
    }
  };

  Op op;
  return op.processVector(geog);
}

// [[Rcpp::export]]
NumericVector cpp_s2_y(List geog) {
  class Op: public UnaryGeographyOperator<NumericVector, double> {
    double processFeature(XPtr<Geography> feature, R_xlen_t i) {
      return feature->Y();
    }
  };

  Op op;
  return op.processVector(geog);
}

// [[Rcpp::export]]
NumericVector cpp_s2_distance(List geog1, List geog2) {
  class Op: public BinaryGeographyOperator<NumericVector, double> {

    double processFeature(XPtr<Geography> feature1,
                          XPtr<Geography> feature2,
                          R_xlen_t i) {
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
NumericVector cpp_s2_max_distance(List geog1, List geog2) {
  class Op: public BinaryGeographyOperator<NumericVector, double> {

    double processFeature(XPtr<Geography> feature1,
                          XPtr<Geography> feature2,
                          R_xlen_t i) {
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
