
#include "s2/s2boolean_operation.h"
#include "s2/s2closest_edge_query.h"
#include "s2/s2latlng_rect.h"
#include "s2/s2polygon.h"
#include "s2/s2builderutil_snap_functions.h"
#include "s2/s2shape_index_buffered_region.h"

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
    int processFeature(XPtr<RGeography> feature1, XPtr<RGeography> feature2, R_xlen_t i) {
      return s2geography::s2_intersects(feature1->Index(), feature2->Index(), options);
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
    int processFeature(XPtr<RGeography> feature1, XPtr<RGeography> feature2, R_xlen_t i) {
      return s2geography::s2_equals(feature1->Index(), feature2->Index(), options);
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
    int processFeature(XPtr<RGeography> feature1, XPtr<RGeography> feature2, R_xlen_t i) {
      return s2geography::s2_contains(feature1->Index(), feature2->Index(), options);
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

    int processFeature(XPtr<RGeography> feature1, XPtr<RGeography> feature2, R_xlen_t i) {
      return s2geography::s2_intersects(feature1->Index(), feature2->Index(), this->closedOptions) &&
        !s2geography::s2_intersects(feature1->Index(), feature2->Index(), this->openOptions);
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
    RGeography* geog2_id;
    std::unique_ptr<S2ClosestEdgeQuery> query;

    Op(NumericVector distance): distance(distance), geog2_id(nullptr) {}

    int processFeature(XPtr<RGeography> feature1, XPtr<RGeography> feature2, R_xlen_t i) {
      if (feature2.get() != geog2_id) {
        this->query = absl::make_unique<S2ClosestEdgeQuery>(&feature2->Index().ShapeIndex());
        this->geog2_id = feature2.get();
      }

      S2ClosestEdgeQuery::ShapeIndexTarget target(&feature1->Index().ShapeIndex());
      return query->IsDistanceLessOrEqual(&target, S1ChordAngle::Radians(this->distance[i]));
    }
  };

  Op op(distance);
  return op.processVector(geog1, geog2);
}

// [[Rcpp::export]]
LogicalVector cpp_s2_prepared_dwithin(List geog1, List geog2, NumericVector distance) {
  if (distance.size() != geog1.size())  {
    stop("Incompatible lengths"); // #nocov
  }

  class Op: public BinaryGeographyOperator<LogicalVector, int> {
  public:
    NumericVector distance;
    S2RegionCoverer coverer;
    std::vector<S2CellId> covering;
    RGeography* covering_id;
    std::unique_ptr<S2ClosestEdgeQuery> query;
    MutableS2ShapeIndex::Iterator iterator;

    Op(NumericVector distance):
      distance(distance), covering_id(nullptr) {}

    int processFeature(XPtr<RGeography> feature1, XPtr<RGeography> feature2, R_xlen_t i) {
      S1ChordAngle distance_angle = S1ChordAngle::Radians(this->distance[i]);

      // Update the query and covering on y if needed
      if (feature2.get() != covering_id) {
        S2ShapeIndexBufferedRegion buffered(&feature2->Index().ShapeIndex(), distance_angle);
        coverer.GetCovering(buffered, &covering);
        this->query = absl::make_unique<S2ClosestEdgeQuery>(&feature2->Index().ShapeIndex());
        this->covering_id = feature2.get();
      }

      // Check for a possible intersection
      iterator.Init(&feature1->Index().ShapeIndex());
      bool may_intersect_buffer = false;
      for (const S2CellId& query_cell: covering) {
          if (iterator.Locate(query_cell) != S2CellRelation::DISJOINT) {
            may_intersect_buffer = true;
            break;
          }
      }

      if (may_intersect_buffer) {
        S2ClosestEdgeQuery::ShapeIndexTarget target(&feature1->Index().ShapeIndex());
        return query->IsDistanceLessOrEqual(&target, distance_angle);
      } else {
        return false;
      }
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

    int processFeature(XPtr<RGeography> feature, R_xlen_t i) {
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

      return s2geography::s2_intersects_box(feature->Index(), rect, options, deltaDegrees);
    }
  };

  Op op(lng1, lat1, lng2, lat2, detail, s2options);
  return op.processVector(geog);
}
