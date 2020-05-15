
#include "libs2-geography-operator.h"
#include "s2/s2closest_edge_query.h"
#include "s2/s2furthest_edge_query.h"
#include <Rcpp.h>
using namespace Rcpp;

// [[Rcpp::export]]
LogicalVector libs2_cpp_s2_iscollection(List geog) {
  class LibS2Op: public LibS2UnaryGeographyOperator<LogicalVector, int> {
    int processFeature(XPtr<LibS2Geography> feature) {
      return feature->IsCollection();
    }
  };

  LibS2Op op;
  return op.processVector(geog);
}

// [[Rcpp::export]]
IntegerVector libs2_cpp_s2_dimension(List geog) {
  class LibS2Op: public LibS2UnaryGeographyOperator<IntegerVector, int> {
    int processFeature(XPtr<LibS2Geography> feature) {
      return feature->Dimension();
    }
  };

  LibS2Op op;
  return op.processVector(geog);
}

// [[Rcpp::export]]
IntegerVector libs2_cpp_s2_numpoints(List geog) {
  class LibS2Op: public LibS2UnaryGeographyOperator<IntegerVector, int> {
    int processFeature(XPtr<LibS2Geography> feature) {
      return feature->NumPoints();
    }
  };

  LibS2Op op;
  return op.processVector(geog);
}

// [[Rcpp::export]]
NumericVector libs2_cpp_s2_area(List geog) {
  class LibS2Op: public LibS2UnaryGeographyOperator<NumericVector, double> {
    double processFeature(XPtr<LibS2Geography> feature) {
      return feature->Area();
    }
  };

  LibS2Op op;
  return op.processVector(geog);
}

// [[Rcpp::export]]
NumericVector libs2_cpp_s2_length(List geog) {
  class LibS2Op: public LibS2UnaryGeographyOperator<NumericVector, double> {
    double processFeature(XPtr<LibS2Geography> feature) {
      return feature->Length();
    }
  };

  LibS2Op op;
  return op.processVector(geog);
}

// [[Rcpp::export]]
NumericVector libs2_cpp_s2_perimeter(List geog) {
  class LibS2Op: public LibS2UnaryGeographyOperator<NumericVector, double> {
    double processFeature(XPtr<LibS2Geography> feature) {
      return feature->Perimeter();
    }
  };

  LibS2Op op;
  return op.processVector(geog);
}

// [[Rcpp::export]]
NumericVector libs2_cpp_s2_x(List geog) {
  class LibS2Op: public LibS2UnaryGeographyOperator<NumericVector, double> {
    double processFeature(XPtr<LibS2Geography> feature) {
      return feature->X();
    }
  };

  LibS2Op op;
  return op.processVector(geog);
}

// [[Rcpp::export]]
NumericVector libs2_cpp_s2_y(List geog) {
  class LibS2Op: public LibS2UnaryGeographyOperator<NumericVector, double> {
    double processFeature(XPtr<LibS2Geography> feature) {
      return feature->Y();
    }
  };

  LibS2Op op;
  return op.processVector(geog);
}

// [[Rcpp::export]]
NumericVector libs2_cpp_s2_distance(List geog1, List geog2) {
  class LibS2Op: public LibS2BinaryGeographyOperator<NumericVector, double> {

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

// [[Rcpp::export]]
NumericVector libs2_cpp_s2_maxdistance(List geog1, List geog2) {
  class LibS2Op: public LibS2BinaryGeographyOperator<NumericVector, double> {

    double processFeature(XPtr<LibS2Geography> feature1, XPtr<LibS2Geography> feature2) {
      S2FurthestEdgeQuery query(feature1->ShapeIndex());
      S2FurthestEdgeQuery::ShapeIndexTarget target(feature2->ShapeIndex());

      const auto& result = query.FindFurthestEdge(&target);

      S1ChordAngle angle = result.distance();
      double distance = angle.ToAngle().radians();

      // returns -1 if one of the indexes is empty
      // -Inf is more consistent with the minimum distance
      // which returns Inf if one of the indexes is empty
      if (distance == -1) {
        distance = R_NegInf;
      }

      return distance;
    }
  };

  LibS2Op op;
  return op.processVector(geog1, geog2);
}
