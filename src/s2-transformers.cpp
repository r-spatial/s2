
#include "s2/s2boolean_operation.h"
#include "s2/s2closest_edge_query.h"
#include "s2/s2polygon.h"
#include "s2/s2polyline.h"
#include "s2/s2point.h"
#include "s2/s2error.h"
#include "s2/s2boolean_operation.h"
#include "s2/s2builder.h"
#include "s2/s2builderutil_s2polygon_layer.h"
#include "s2/s2builderutil_s2polyline_vector_layer.h"
#include "s2/s2builderutil_s2point_vector_layer.h"
#include "s2/s2builderutil_closed_set_normalizer.h"
#include "s2/s2builderutil_snap_functions.h"
#include "s2/s2shape_index_buffered_region.h"
#include "s2/s2region_coverer.h"

#include "s2-options.h"
#include "geography-operator.h"
#include "point-geography.h"
#include "polyline-geography.h"
#include "polygon-geography.h"
#include "geography-collection.h"

#include <Rcpp.h>
using namespace Rcpp;

std::unique_ptr<Geography> geographyFromLayers(std::vector<S2Point> points,
                                               std::vector<std::unique_ptr<S2Polyline>> polylines,
                                               std::unique_ptr<S2Polygon> polygon) {
  // count non-empty dimensions
  int nonEmptyDimensions = (!polygon->is_empty() + (polylines.size() > 0) + (points.size() > 0));

  // return empty output
  if (nonEmptyDimensions == 0) {
    return absl::make_unique<GeographyCollection>();
  }

  // return mixed dimension output
  if (nonEmptyDimensions > 1) {
    std::vector<std::unique_ptr<Geography>> features;

    if (points.size() > 0) {
      features.push_back(absl::make_unique<PointGeography>(std::move(points)));
    }

    if (polylines.size() > 0) {
      features.push_back(absl::make_unique<PolylineGeography>(std::move(polylines)));
    }

    if (!polygon->is_empty()) {
      features.push_back(absl::make_unique<PolygonGeography>(std::move(polygon)));
    }

    return absl::make_unique<GeographyCollection>(std::move(features));
  }

  // return single dimension output
  if (!polygon->is_empty()) {
    return absl::make_unique<PolygonGeography>(std::move(polygon));
  } else if (polylines.size() > 0) {
    return absl::make_unique<PolylineGeography>(std::move(polylines));
  } else {
    return absl::make_unique<PointGeography>(std::move(points));
  }
}

std::unique_ptr<Geography> doBooleanOperation(S2ShapeIndex* index1, S2ShapeIndex* index2,
                                              S2BooleanOperation::OpType opType,
                                              S2BooleanOperation::Options options) {

  // create the data structures that will contain the output
  std::vector<S2Point> points;
  std::vector<std::unique_ptr<S2Polyline>> polylines;
  std::unique_ptr<S2Polygon> polygon = absl::make_unique<S2Polygon>();

  s2builderutil::LayerVector layers(3);
  // currently using the default S2PointVectorLayer::Options
  layers[0] = absl::make_unique<s2builderutil::S2PointVectorLayer>(&points);
  // currently using the default S2PolylineVectorLayer::Options
  layers[1] = absl::make_unique<s2builderutil::S2PolylineVectorLayer>(&polylines);
  // currently using the default S2PolygonLayer::Options
  layers[2] = absl::make_unique<s2builderutil::S2PolygonLayer>(polygon.get());

  // do the boolean operation
  S2BooleanOperation booleanOp(
    opType,
    // normalizing the closed set here is required for line intersections
    // to work as expected
    s2builderutil::NormalizeClosedSet(std::move(layers)),
    options
  );

  // check for errors
  S2Error error;
  if (!booleanOp.Build(*index1, *index2, &error)) {
    stop(error.text());
  }

  // construct output
  return geographyFromLayers(
    std::move(points),
    std::move(polylines),
    std::move(polygon)
  );
}

class BooleanOperationOp: public BinaryGeographyOperator<List, SEXP> {
public:
  BooleanOperationOp(S2BooleanOperation::OpType opType, List s2options):
    opType(opType) {
      GeographyOperationOptions options(s2options);
      this->options = options.booleanOperationOptions();
    }

  SEXP processFeature(XPtr<Geography> feature1, XPtr<Geography> feature2, R_xlen_t i) {
    std::unique_ptr<Geography> geography = doBooleanOperation(
      feature1->ShapeIndex(),
      feature2->ShapeIndex(),
      this->opType,
      this->options
    );

    return Rcpp::XPtr<Geography>(geography.release());
  }

private:
  S2BooleanOperation::OpType opType;
  S2BooleanOperation::Options options;
};

// [[Rcpp::export]]
List cpp_s2_intersection(List geog1, List geog2, List s2options) {
  BooleanOperationOp op(S2BooleanOperation::OpType::INTERSECTION, s2options);
  return op.processVector(geog1, geog2);
}

// [[Rcpp::export]]
List cpp_s2_union(List geog1, List geog2, List s2options) {
  BooleanOperationOp op(S2BooleanOperation::OpType::UNION, s2options);
  return op.processVector(geog1, geog2);
}

// [[Rcpp::export]]
List cpp_s2_difference(List geog1, List geog2, List s2options) {
  BooleanOperationOp op(S2BooleanOperation::OpType::DIFFERENCE, s2options);
  return op.processVector(geog1, geog2);
}

// [[Rcpp::export]]
List cpp_s2_sym_difference(List geog1, List geog2, List s2options) {
  BooleanOperationOp op(S2BooleanOperation::OpType::SYMMETRIC_DIFFERENCE, s2options);
  return op.processVector(geog1, geog2);
}

// [[Rcpp::export]]
List cpp_s2_union_agg(List geog, List s2options, bool naRm) {
  GeographyOperationOptions options(s2options);

  MutableS2ShapeIndex index;
  SEXP item;
  for (R_xlen_t i = 0; i < geog.size(); i++) {
    item = geog[i];
    if (item == R_NilValue && !naRm) {
      return List::create(R_NilValue);
    }

    if (item != R_NilValue) {
      Rcpp::XPtr<Geography> feature(item);
      feature->BuildShapeIndex(&index);
    }
  }

  MutableS2ShapeIndex emptyIndex;
  std::unique_ptr<Geography> geography = doBooleanOperation(
    &index,
    &emptyIndex,
    S2BooleanOperation::OpType::UNION,
    options.booleanOperationOptions()
  );

  return List::create(Rcpp::XPtr<Geography>(geography.release()));
}

// [[Rcpp::export]]
List cpp_s2_centroid_agg(List geog, bool naRm) {
  S2Point cumCentroid;

  SEXP item;
  for (R_xlen_t i = 0; i < geog.size(); i++) {
    item = geog[i];
    if (item == R_NilValue && !naRm) {
      return List::create(R_NilValue);
    }

    if (item != R_NilValue) {
      Rcpp::XPtr<Geography> feature(item);
      S2Point centroid = feature->Centroid();
      if (centroid.Norm2() > 0) {
        cumCentroid += centroid.Normalize();
      }
    }
  }

  List output(1);
  if (cumCentroid.Norm2() == 0) {
    output[0] = Rcpp::XPtr<Geography>(new PointGeography());
  } else {
    output[0] = Rcpp::XPtr<Geography>(new PointGeography(cumCentroid));
  }

  return output;
}

std::vector<S2Point> findClosestPoints(S2ShapeIndex* index1, S2ShapeIndex* index2) {
      // see http://s2geometry.io/devguide/s2closestedgequery.html section on Modeling Accuracy:

      // Find the edge from index2 that is closest to index1
      S2ClosestEdgeQuery query1(index1);
      query1.mutable_options()->set_include_interiors(false);
      S2ClosestEdgeQuery::ShapeIndexTarget target1(index2);
      auto result1 = query1.FindClosestEdge(&target1);

      if (result1.edge_id() == -1) {
        return std::vector<S2Point>();
      }

      // Get the edge from index1 (edge1) that is closest to index2.
      S2Shape::Edge edge1 = query1.GetEdge(result1);

      // Now find the edge from index2 (edge2) that is closest to edge1.
      S2ClosestEdgeQuery query2(index2);
      query2.mutable_options()->set_include_interiors(false);
      S2ClosestEdgeQuery::EdgeTarget target2(edge1.v0, edge1.v1);
      auto result2 = query2.FindClosestEdge(&target2);

      // what if result2 has no edges?
      if (result2.is_interior()) {
        stop("S2ClosestEdgeQuery result is interior!");
      }
      S2Shape::Edge edge2 = query2.GetEdge(result2);

      // Find the closest point pair on edge1 and edge2.
      std::pair<S2Point, S2Point> closest = S2::GetEdgePairClosestPoints(
        edge1.v0, edge1.v1,
        edge2.v0, edge2.v1
      );

      std::vector<S2Point> pts(2);
      pts[0] = closest.first;
      pts[1] = closest.second;
      return pts;
}

// [[Rcpp::export]]
List cpp_s2_closest_point(List geog1, List geog2) {
  class Op: public BinaryGeographyOperator<List, SEXP> {

    SEXP processFeature(XPtr<Geography> feature1, XPtr<Geography> feature2, R_xlen_t i) {
      std::vector<S2Point> pts = findClosestPoints(feature1->ShapeIndex(), feature2->ShapeIndex());

      if (pts.size() == 0) {
        return XPtr<Geography>(new PointGeography());
      } else {
        return XPtr<Geography>(new PointGeography(pts[0]));
      }
    }
  };

  Op op;
  return op.processVector(geog1, geog2);
}

// [[Rcpp::export]]
List cpp_s2_minimum_clearance_line_between(List geog1, List geog2) {
  class Op: public BinaryGeographyOperator<List, SEXP> {

    SEXP processFeature(XPtr<Geography> feature1, XPtr<Geography> feature2, R_xlen_t i) {
      std::vector<S2Point> pts = findClosestPoints(feature1->ShapeIndex(), feature2->ShapeIndex());

      if (pts.size() == 0) {
        return XPtr<Geography>(new PolylineGeography());
      } else if (pts[0] == pts[1]) {
        return XPtr<Geography>(new PointGeography(pts));
      } else {
        std::unique_ptr<S2Polyline> polyline = absl::make_unique<S2Polyline>();
        polyline->Init(pts);
        std::vector<std::unique_ptr<S2Polyline>> polylines(1);
        polylines[0] = std::move(polyline);
        return XPtr<Geography>(new PolylineGeography(std::move(polylines)));
      }
    }
  };

  Op op;
  return op.processVector(geog1, geog2);
}

// [[Rcpp::export]]
List cpp_s2_centroid(List geog) {
  class Op: public UnaryGeographyOperator<List, SEXP> {
    SEXP processFeature(XPtr<Geography> feature, R_xlen_t i) {
      S2Point centroid = feature->Centroid();
      if (centroid.Norm2() == 0) {
        return XPtr<Geography>(new PointGeography());
      } else {
        return XPtr<Geography>(new PointGeography(centroid.Normalize()));
      }
    }
  };

  Op op;
  return op.processVector(geog);
}

// [[Rcpp::export]]
List cpp_s2_boundary(List geog) {
  class Op: public UnaryGeographyOperator<List, SEXP> {
    SEXP processFeature(XPtr<Geography> feature, R_xlen_t i) {
      std::unique_ptr<Geography> ptr = feature->Boundary();
      return XPtr<Geography>(ptr.release());
    }
  };

  Op op;
  return op.processVector(geog);
}


// [[Rcpp::export]]
List cpp_s2_buffer_cells(List geog, NumericVector distance, int maxCells, int minLevel) {
  class Op: public UnaryGeographyOperator<List, SEXP> {
  public:
    NumericVector distance;
    S2RegionCoverer coverer;

    Op(NumericVector distance, int maxCells, int minLevel): distance(distance) {
      this->coverer.mutable_options()->set_max_cells(maxCells);
      if (minLevel > 0) {
        this->coverer.mutable_options()->set_min_level(minLevel);
      }
    }

    SEXP processFeature(XPtr<Geography> feature, R_xlen_t i) {
      S2ShapeIndexBufferedRegion region;
      region.Init(feature->ShapeIndex(), S1ChordAngle::Radians(this->distance[i]));

      S2CellUnion cellUnion;
      cellUnion = coverer.GetCovering(region);

      std::unique_ptr<S2Polygon> polygon = absl::make_unique<S2Polygon>();
      polygon->InitToCellUnionBorder(cellUnion);

      return XPtr<PolygonGeography>(new PolygonGeography(std::move(polygon)));
    }
  };

  Op op(distance, maxCells, minLevel);
  return op.processVector(geog);
}
