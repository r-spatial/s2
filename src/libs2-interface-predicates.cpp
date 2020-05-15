
#include "libs2-s2geography.h"
#include "libs2-geography-operator.h"
#include "s2/s2boolean_operation.h"
#include "s2/s2closest_edge_query.h"

#include <Rcpp.h>
using namespace Rcpp;

// [[Rcpp::export]]
LogicalVector libs2_cpp_s2_intersects(List geog1, List geog2) {
  class LibS2Op: public LibS2BinaryGeographyOperator<LogicalVector, int> {

    int processFeature(XPtr<LibS2Geography> feature1, XPtr<LibS2Geography> feature2) {
      return S2BooleanOperation::Intersects(*feature1->ShapeIndex(), *feature2->ShapeIndex());
    }
  };

  LibS2Op op;
  return op.processVector(geog1, geog2);
}

// [[Rcpp::export]]
LogicalVector libs2_cpp_s2_equals(List geog1, List geog2) {
  class LibS2Op: public LibS2BinaryGeographyOperator<LogicalVector, int> {

    int processFeature(XPtr<LibS2Geography> feature1, XPtr<LibS2Geography> feature2) {
      return S2BooleanOperation::Equals(*feature1->ShapeIndex(), *feature2->ShapeIndex());
    }
  };

  LibS2Op op;
  return op.processVector(geog1, geog2);
}

// [[Rcpp::export]]
LogicalVector libs2_cpp_s2_contains(List geog1, List geog2) {
  class LibS2Op: public LibS2BinaryGeographyOperator<LogicalVector, int> {

    int processFeature(XPtr<LibS2Geography> feature1, XPtr<LibS2Geography> feature2) {
      return S2BooleanOperation::Contains(*feature1->ShapeIndex(), *feature2->ShapeIndex());
    }
  };

  LibS2Op op;
  return op.processVector(geog1, geog2);
}

// [[Rcpp::export]]
LogicalVector libs2_cpp_s2_dwithin(List geog1, List geog2, double distance) {

  class LibS2Op: public LibS2BinaryGeographyOperator<LogicalVector, int> {
  public:
    double distance;
    LibS2Op(double distance): distance(distance) {}

    int processFeature(XPtr<LibS2Geography> feature1, XPtr<LibS2Geography> feature2) {
      S2ClosestEdgeQuery query(feature1->ShapeIndex());
      S2ClosestEdgeQuery::ShapeIndexTarget target(feature2->ShapeIndex());
      return query.IsDistanceLessOrEqual(&target, S1ChordAngle::Radians(this->distance));
    }
  };

  LibS2Op op(distance);
  return op.processVector(geog1, geog2);
}
