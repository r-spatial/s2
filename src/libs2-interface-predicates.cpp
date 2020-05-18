
#include "libs2-s2geography.h"
#include "libs2-geography-operator.h"
#include "s2/s2boolean_operation.h"
#include "s2/s2closest_edge_query.h"
#include "s2/s2latlng_rect.h"

#include <Rcpp.h>
using namespace Rcpp;

// [[Rcpp::export]]
LogicalVector libs2_cpp_s2_intersects(List geog1, List geog2) {
  class LibS2Op: public LibS2BinaryGeographyOperator<LogicalVector, int> {

    int processFeature(XPtr<LibS2Geography> feature1, XPtr<LibS2Geography> feature2, R_xlen_t i) {
      return S2BooleanOperation::Intersects(*feature1->ShapeIndex(), *feature2->ShapeIndex());
    }
  };

  LibS2Op op;
  return op.processVector(geog1, geog2);
}

// [[Rcpp::export]]
LogicalVector libs2_cpp_s2_equals(List geog1, List geog2) {
  class LibS2Op: public LibS2BinaryGeographyOperator<LogicalVector, int> {

    int processFeature(XPtr<LibS2Geography> feature1, XPtr<LibS2Geography> feature2, R_xlen_t i) {
      return S2BooleanOperation::Equals(*feature1->ShapeIndex(), *feature2->ShapeIndex());
    }
  };

  LibS2Op op;
  return op.processVector(geog1, geog2);
}

// [[Rcpp::export]]
LogicalVector libs2_cpp_s2_contains(List geog1, List geog2) {
  class LibS2Op: public LibS2BinaryGeographyOperator<LogicalVector, int> {

    int processFeature(XPtr<LibS2Geography> feature1, XPtr<LibS2Geography> feature2, R_xlen_t i) {
      return S2BooleanOperation::Contains(*feature1->ShapeIndex(), *feature2->ShapeIndex());
    }
  };

  LibS2Op op;
  return op.processVector(geog1, geog2);
}

// [[Rcpp::export]]
LogicalVector libs2_cpp_s2_dwithin(List geog1, List geog2, NumericVector distance) {
  if (distance.size() != geog1.size())  {
    stop("Incompatible lengths");
  }

  class LibS2Op: public LibS2BinaryGeographyOperator<LogicalVector, int> {
  public:
    NumericVector distance;
    LibS2Op(NumericVector distance): distance(distance) {}

    int processFeature(XPtr<LibS2Geography> feature1, XPtr<LibS2Geography> feature2, R_xlen_t i) {
      S2ClosestEdgeQuery query(feature1->ShapeIndex());
      S2ClosestEdgeQuery::ShapeIndexTarget target(feature2->ShapeIndex());
      return query.IsDistanceLessOrEqual(&target, S1ChordAngle::Radians(this->distance[i]));
    }
  };

  LibS2Op op(distance);
  return op.processVector(geog1, geog2);
}

// [[Rcpp::export]]
LogicalVector libs2_cpp_s2_intersectsbox(List geog,
                                         NumericVector lng1, NumericVector lat1,
                                         NumericVector lng2, NumericVector lat2) {

  class LibS2Op: public LibS2UnaryGeographyOperator<LogicalVector, int> {
  public:
    NumericVector lng1, lat1, lng2, lat2;

    LibS2Op(NumericVector lng1, NumericVector lat1,
            NumericVector lng2, NumericVector lat2):
      lng1(lng1), lat1(lat1), lng2(lng2), lat2(lat2) {}

    int processFeature(XPtr<LibS2Geography> feature, R_xlen_t i) {
      // construct rect
      S2LatLng lo = S2LatLng::FromDegrees(this->lng1[i], this->lat1[i]);
      S2LatLng hi = S2LatLng::FromDegrees(this->lng2[i], this->lat2[i]);
      S2LatLngRect rect(lo, hi);

      // Incomplete and conservative
      // (may return true when the lat/lon rect doesn't actually contain the geography)
      MutableS2ShapeIndex* index = (MutableS2ShapeIndex*)feature->ShapeIndex();
      for (MutableS2ShapeIndex::Iterator it(index, S2ShapeIndex::BEGIN); !it.done(); it.Next()) {
        if (rect.Intersects(S2Cell(it.id()))) {
          const S2ShapeIndexCell& cell = it.cell();
          for (int j = 0; j < cell.num_clipped(); j++) {
            // const S2ClippedShape& clippedShape = cell.clipped(j);
            // S2Shape* shape = index->shape(clippedShape.shape_id());
            // don't know what to do with shape here to properly test for intersection
          }
          return true;
        }
      }

      return false;
    }
  };

  LibS2Op op(lng1, lat1, lng2, lat2);
  return op.processVector(geog);
}

