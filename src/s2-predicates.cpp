
#include "s2/s2boolean_operation.h"
#include "s2/s2closest_edge_query.h"
#include "s2/s2latlng_rect.h"
#include "s2/s2polygon.h"
#include "s2/s2testing.h"
#include "s2/s2builderutil_snap_functions.h"

#include "geography-operator.h"
#include "s2-options.h"

#include <Rcpp.h>
using namespace Rcpp;

class BinaryPredicateOperator: public BinaryGeographyOperator<LogicalVector, int> {
public:
  S2BooleanOperation::Options options;

  BinaryPredicateOperator(List s2options) {
    GeographyOperationOptions options(s2options);
    this->options = options.booleanOperationOptions();
  }
};

// [[Rcpp::export]]
LogicalVector cpp_s2_intersects(List geog1, List geog2, List s2options) {
  class Op: public BinaryPredicateOperator {
  public:
    Op(List s2options): BinaryPredicateOperator(s2options) {}
    int processFeature(XPtr<Geography> feature1, XPtr<Geography> feature2, R_xlen_t i) {
      auto geog1 = feature1->NewGeography();
      auto geog2 = feature2->NewGeography();
      s2geography::S2GeographyShapeIndex index1(*geog1);
      s2geography::S2GeographyShapeIndex index2(*geog2);

      return s2geography::s2_intersects(index1, index2, options);
    };
  };

  Op op(s2options);
  return op.processVector(geog1, geog2);
}

// [[Rcpp::export]]
LogicalVector cpp_s2_equals(List geog1, List geog2, List s2options) {
  // for s2_equals(), handling polygon_model wouldn't make sense, right?
  class Op: public BinaryPredicateOperator {
  public:
    Op(List s2options): BinaryPredicateOperator(s2options) {}
    int processFeature(XPtr<Geography> feature1, XPtr<Geography> feature2, R_xlen_t i) {
      auto geog1 = feature1->NewGeography();
      auto geog2 = feature2->NewGeography();
      s2geography::S2GeographyShapeIndex index1(*geog1);
      s2geography::S2GeographyShapeIndex index2(*geog2);

      return s2geography::s2_equals(index1, index2, options);
    }
  };

  Op op(s2options);
  return op.processVector(geog1, geog2);
}

// [[Rcpp::export]]
LogicalVector cpp_s2_contains(List geog1, List geog2, List s2options) {
  class Op: public BinaryPredicateOperator {
  public:
    Op(List s2options): BinaryPredicateOperator(s2options) {}
    int processFeature(XPtr<Geography> feature1, XPtr<Geography> feature2, R_xlen_t i) {
      auto geog1 = feature1->NewGeography();
      auto geog2 = feature2->NewGeography();
      s2geography::S2GeographyShapeIndex index1(*geog1);
      s2geography::S2GeographyShapeIndex index2(*geog2);
      return s2geography::s2_contains(index1, index2, options);
    }
  };

  Op op(s2options);
  return op.processVector(geog1, geog2);
}

// [[Rcpp::export]]
LogicalVector cpp_s2_touches(List geog1, List geog2, List s2options) {
  class Op: public BinaryPredicateOperator {
  public:
    Op(List s2options): BinaryPredicateOperator(s2options) {
      this->closedOptions = this->options;
      this->closedOptions.set_polygon_model(S2BooleanOperation::PolygonModel::CLOSED);
      this->closedOptions.set_polyline_model(S2BooleanOperation::PolylineModel::CLOSED);

      this->openOptions = this->options;
      this->openOptions.set_polygon_model(S2BooleanOperation::PolygonModel::OPEN);
      this->openOptions.set_polyline_model(S2BooleanOperation::PolylineModel::OPEN);
    }

    int processFeature(XPtr<Geography> feature1, XPtr<Geography> feature2, R_xlen_t i) {
      auto geog1 = feature1->NewGeography();
      auto geog2 = feature2->NewGeography();
      s2geography::S2GeographyShapeIndex index1(*geog1);
      s2geography::S2GeographyShapeIndex index2(*geog2);

      return s2geography::s2_intersects(index1, index2, this->closedOptions) &&
        !s2geography::s2_intersects(index1, index2, this->openOptions);
    }

  private:
    S2BooleanOperation::Options closedOptions;
    S2BooleanOperation::Options openOptions;
  };

  Op op(s2options);
  return op.processVector(geog1, geog2);
}

// [[Rcpp::export]]
LogicalVector cpp_s2_dwithin(List geog1, List geog2, NumericVector distance) {
  if (distance.size() != geog1.size())  {
    stop("Incompatible lengths"); // #nocov
  }

  class Op: public BinaryGeographyOperator<LogicalVector, int> {
  public:
    NumericVector distance;
    Op(NumericVector distance): distance(distance) {}

    int processFeature(XPtr<Geography> feature1, XPtr<Geography> feature2, R_xlen_t i) {
      S2ClosestEdgeQuery query(feature1->ShapeIndex());
      S2ClosestEdgeQuery::ShapeIndexTarget target(feature2->ShapeIndex());
      return query.IsDistanceLessOrEqual(&target, S1ChordAngle::Radians(this->distance[i]));
    }
  };

  Op op(distance);
  return op.processVector(geog1, geog2);
}

// [[Rcpp::export]]
LogicalVector cpp_s2_intersects_box(List geog,
                                    NumericVector lng1, NumericVector lat1,
                                    NumericVector lng2, NumericVector lat2,
                                    IntegerVector detail,
                                    List s2options) {

  class Op: public UnaryGeographyOperator<LogicalVector, int> {
  public:
    NumericVector lng1, lat1, lng2, lat2;
    IntegerVector detail;
    S2BooleanOperation::Options options;

    Op(NumericVector lng1, NumericVector lat1,
       NumericVector lng2, NumericVector lat2,
       IntegerVector detail, List s2options):
      lng1(lng1), lat1(lat1), lng2(lng2), lat2(lat2), detail(detail) {

      GeographyOperationOptions options(s2options);
      this->options = options.booleanOperationOptions();
    }

    int processFeature(XPtr<Geography> feature, R_xlen_t i) {
      // construct polygon
      // this might be easier with an s2region intersection
      double xmin = this->lng1[i];
      double ymin = this->lat1[i];
      double xmax = this->lng2[i];
      double ymax = this->lat2[i];
      int detail = this->detail[i];

      if (detail < 1) {
        stop("Can't create polygon from bounding box with detail < 1");
      }

      // can't just do xmax - xmin because these boxes can wrap around the date line
      S2Point westEquator = S2LatLng::FromDegrees(0, xmin).Normalized().ToPoint();
      S2Point eastEquator = S2LatLng::FromDegrees(0, xmax).Normalized().ToPoint();
      S1ChordAngle width(westEquator, eastEquator);
      double widthDegrees = width.degrees();
      double deltaDegrees = widthDegrees / (double) detail;
      double heightDegrees = ymax - ymin;

      // these situations would result in an error below because of
      // duplicate vertices
      if (widthDegrees == 0 || heightDegrees == 0) {
        return false;
      }

      S2LatLngRect rect(S2LatLng::FromDegrees(ymin, xmin), S2LatLng::FromDegrees(ymax, xmax));

      auto geog = feature->NewGeography();
      s2geography::S2GeographyShapeIndex index(*geog);
      return s2geography::s2_intersects_box(index, rect, options, deltaDegrees);
    }
  };

  Op op(lng1, lat1, lng2, lat2, detail, s2options);
  return op.processVector(geog);
}
