
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

#include "model.h"
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

Rcpp::XPtr<Geography> doBooleanOperation(S2ShapeIndex* index1, S2ShapeIndex* index2,
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
  S2Error error;
  if (!booleanOp.Build(*index1, *index2, &error)) {
    stop(error.text()); // # nocov
  }

  // construct output
  std::unique_ptr<Geography> geography = geographyFromLayers(
    std::move(points), 
    std::move(polylines), 
    std::move(polygon)
  );

  // return XPtr
  return Rcpp::XPtr<Geography>(geography.release());
}

class BooleanOperationOp: public BinaryGeographyOperator<List, SEXP> {
public:
  BooleanOperationOp(S2BooleanOperation::OpType opType, int model, int snapLevel):
    opType(opType) {

    if (model >= 0) {
      this->options.set_polygon_model(get_polygon_model(model));
      this->options.set_polyline_model(get_polyline_model(model));
    }
    if (snapLevel > 0) {
      this->options.set_snap_function(s2builderutil::S2CellIdSnapFunction(snapLevel));
    }
  }

  SEXP processFeature(XPtr<Geography> feature1, XPtr<Geography> feature2, R_xlen_t i) {
    return doBooleanOperation(feature1->ShapeIndex(), feature2->ShapeIndex(), this->opType, this->options);
  }

private:
  S2BooleanOperation::OpType opType;
  S2BooleanOperation::Options options;
};

// [[Rcpp::export]]
List cpp_s2_intersection(List geog1, List geog2, int model, int snapLevel) {
  BooleanOperationOp op(S2BooleanOperation::OpType::INTERSECTION, model, snapLevel);
  return op.processVector(geog1, geog2);
}

// [[Rcpp::export]]
List cpp_s2_union(List geog1, List geog2, int model, int snapLevel) {
  BooleanOperationOp op(S2BooleanOperation::OpType::UNION, model, snapLevel);
  return op.processVector(geog1, geog2);
}

// [[Rcpp::export]]
List cpp_s2_difference(List geog1, List geog2, int model, int snapLevel) {
  BooleanOperationOp op(S2BooleanOperation::OpType::DIFFERENCE, model, snapLevel);
  return op.processVector(geog1, geog2);
}

// [[Rcpp::export]]
List cpp_s2_sym_difference(List geog1, List geog2, int model, int snapLevel) {
  BooleanOperationOp op(S2BooleanOperation::OpType::SYMMETRIC_DIFFERENCE, model, snapLevel);
  return op.processVector(geog1, geog2);
}

// [[Rcpp::export]]
List cpp_s2_union_agg(List geog, int model, int snapLevel, bool naRm) {
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

  S2BooleanOperation::Options options;
  if (model >= 0) {
    options.set_polygon_model(get_polygon_model(model));
    options.set_polyline_model(get_polyline_model(model));
  }
  if (snapLevel > 0) {
    options.set_snap_function(s2builderutil::S2CellIdSnapFunction(snapLevel));
  }

  List output(1);
  MutableS2ShapeIndex emptyIndex;
  output[0] = doBooleanOperation(&index, &emptyIndex,S2BooleanOperation::OpType::UNION, options);
  return output;
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

// [[Rcpp::export]]
List cpp_s2_closest_point(List geog1, List geog2) {
  class Op: public BinaryGeographyOperator<List, SEXP> {

    SEXP processFeature(XPtr<Geography> feature1, XPtr<Geography> feature2, R_xlen_t i) {
      /*
      S2ClosestEdgeQuery query(feature1->ShapeIndex());
      S2ClosestEdgeQuery::ShapeIndexTarget target(feature2->ShapeIndex());

      const auto& result = query.FindClosestEdge(&target);

      //  result.edge_id() == -1 means there was no match
      if (result.edge_id() == -1) {
        return XPtr<Geography>(new PointGeography());
      }

      // get the edge on feature1 that is closest to feature2
      // the point returned here must be somewhere along this edge
      const S2Shape::Edge edge1 = query.GetEdge(result);

      // the edge on feature 1 *is* a point: easy!
      if (edge1.v0 == edge1.v1) {
        return XPtr<Geography>(new PointGeography(edge1.v0));
      }

      // reverse query: find the edge on feature2 that is closest to feature1
      S2ClosestEdgeQuery reverseQuery(feature2->ShapeIndex());
      S2ClosestEdgeQuery::EdgeTarget reverseTarget(edge1.v0, edge1.v1);
      const auto& reverseResult = reverseQuery.FindClosestEdge(&target);

      // get the edge on feature2 that is closest to feature1
      const S2Shape::Edge edge2 = reverseQuery.GetEdge(reverseResult);

      // the edge on feature 2 *is* a point: sort of easy!
      if (edge2.v0 == edge2.v1) {
        S2Point closest = query.Project(edge2.v0, result);
        return XPtr<Geography>(new PointGeography(closest));
      } else {
        stop("Don't know how to find the closest point given two non-point edges");
      }
      */
      // see http://s2geometry.io/devguide/s2closestedgequery.html section on Modeling Accuracy:

      S2ClosestEdgeQuery query1(feature1->ShapeIndex());
      query1.mutable_options()->set_include_interiors(false);
      S2ClosestEdgeQuery::ShapeIndexTarget target2(feature2->ShapeIndex());
      auto result1 = query1.FindClosestEdge(&target2);
      if (result1.edge_id() == -1) {
        return XPtr<Geography>(new PointGeography());
      }
      // Get the edge from index1 (edge1) that is closest to index2.
      S2Shape::Edge edge1 = query1.GetEdge(result1);

      // Now find the edge from index2 (edge2) that is closest to edge1.
      S2ClosestEdgeQuery query2(feature2->ShapeIndex());
      query2.mutable_options()->set_include_interiors(false);
      S2ClosestEdgeQuery::EdgeTarget target1(edge1.v0, edge1.v1);
      auto result2 = query2.FindClosestEdge(&target1);
      // what if result2 has no edges?
      if (result2.is_interior())
        stop("result is interior!");
      S2Shape::Edge edge2 = query2.GetEdge(result2);

      // Find the closest point pair on edge1 and edge2.
      auto closest = S2::GetEdgePairClosestPoints(edge1.v0, edge1.v1,
                                                  edge2.v0, edge2.v1);
      // meters = GeoidDistance(closest.first, closest.second);
      // return LINESTRING with these two points:

      std::vector<S2Point> pts(2);
      pts[0] = closest.first;
      pts[1] = closest.second;
      if (closest.first == closest.second) {
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
List cpp_s2_nearest_feature(List geog1, List geog2) {
  class Op: public BinaryGeographyOperator<List, SEXP> {

    SEXP processFeature(XPtr<Geography> feature1, XPtr<Geography> feature2, R_xlen_t i) {
      S2ClosestEdgeQuery query(feature1->ShapeIndex());
      query.mutable_options()->set_include_interiors(false);
      S2ClosestEdgeQuery::ShapeIndexTarget target(feature2->ShapeIndex());

      const auto& result = query.FindClosestEdge(&target);
      int s = result.shape_id();
      if (s == -1) // geog1 or geog2 is empty:
        return Rcpp::IntegerVector::create(NA_INTEGER);
      else
        return Rcpp::IntegerVector::create(result.shape_id() + 1); // R: 1-based index
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
