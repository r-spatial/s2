
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

#include "snap.h"
#include "model.h"
#include "geography-operator.h"
#include "point-geography.h"
#include "polyline-geography.h"
#include "polygon-geography.h"
#include "geography-collection.h"

#include <Rcpp.h>
using namespace Rcpp;

template <S2BooleanOperation::OpType opType>
Rcpp::XPtr<Geography> doBooleanOperation(S2ShapeIndex* index1, S2ShapeIndex* index2, 
      S2BooleanOperation::Options options) {

  FLAGS_s2debug = false;

  MutableS2ShapeIndex index;
  s2builderutil::IndexedS2PolylineVectorLayer::Options polyline_options;
  polyline_options.set_edge_type(S2Builder::EdgeType::UNDIRECTED);
  polyline_options.set_polyline_type(S2Builder::Graph::PolylineType::WALK);
  polyline_options.set_duplicate_edges(S2Builder::GraphOptions::DuplicateEdges::MERGE);
  s2builderutil::IndexedS2PolygonLayer::Options polygon_options;
  s2builderutil::LayerVector layers(3);
  layers[0] = absl::make_unique<s2builderutil::IndexedS2PointVectorLayer>(&index);
  layers[1] = absl::make_unique<s2builderutil::IndexedS2PolylineVectorLayer>(
     &index, polyline_options);
  layers[2] = absl::make_unique<s2builderutil::IndexedS2PolygonLayer>(&index, polygon_options);

  S2BooleanOperation op2(opType, s2builderutil::NormalizeClosedSet(std::move(layers)), options);

  S2Error error;
  if (!op2.Build(*index1, *index2, &error)) {
    stop(error.text()); // #nocov
  }

  std::vector<S2Point> points;
  std::vector<std::unique_ptr<S2Polyline>> polylines;
  std::unique_ptr<S2Polygon> polygon = absl::make_unique<S2Polygon>();

  std::vector<std::unique_ptr<Geography>> features;
  std::vector<std::unique_ptr<S2Shape>> shapes(std::move(index.ReleaseAll()));
  int dims = 0; // bitfield with dimension flags in lower 3 bits
  for (int i = 0; i < shapes.size(); i++) {
    std::unique_ptr<S2Shape> shape = std::move(shapes[i]);
    switch (shape->dimension()) {
      case 0: {
        dims = dims | 1;
        std::unique_ptr<S2PointVectorShape> p(static_cast<S2PointVectorShape*>(shape.release()));
        for (int j = 0; j < p->num_points(); j++) 
          points.push_back(p->point(j));
        break;
      }
      case 1: {
        dims = dims | 2;
        std::unique_ptr<S2Polyline::Shape> polyline_shape(static_cast<S2Polyline::Shape*>(shape.release()));
        S2Polyline *clone = polyline_shape->polyline()->Clone();
        std::unique_ptr<S2Polyline> polyline(clone);
        polylines.push_back(std::move(polyline));
        break;
      }
      case 2: {
        dims = dims | 4;
        std::unique_ptr<S2Polygon::Shape> polygon_shape(static_cast<S2Polygon::Shape*>(shape.release()));
        S2Polygon *clone = polygon_shape->polygon()->Clone();
        std::unique_ptr<S2Polygon> polyg(clone);
        polygon = std::move(polyg);
        break;
      }
    }
  }

  int ndims = ((dims & 1) != 0) + ((dims & 2) != 0) + ((dims & 4) != 0);
  switch (ndims) {
    case 0: // EMPTY
      return Rcpp::XPtr<Geography>(new GeographyCollection());
    case 1: { // SINGLE DIM
      if (!polygon->is_empty()) {
        return Rcpp::XPtr<Geography>(new PolygonGeography(std::move(polygon)));
      } else if (polylines.size() > 0) {
        return Rcpp::XPtr<Geography>(new PolylineGeography(std::move(polylines)));
      } else {
        return Rcpp::XPtr<Geography>(new PointGeography(std::move(points)));
      }
      break;
    }
    default: // GEOMETRYCOLLECTION
      if (points.size() > 0) {
        features.push_back(absl::make_unique<PointGeography>(std::move(points)));
      }
      if (polylines.size() > 0) {
        features.push_back(absl::make_unique<PolylineGeography>(std::move(polylines)));
      }
      if (!polygon->is_empty()) {
        features.push_back(absl::make_unique<PolygonGeography>(std::move(polygon)));
      }
      return Rcpp::XPtr<Geography>(new GeographyCollection(std::move(features)));
    break; // never reached;
  }
}

template <S2BooleanOperation::OpType opType, int model>
class BooleanOperationOp: public BinaryGeographyOperator<List, SEXP> {
  SEXP processFeature(XPtr<Geography> feature1, XPtr<Geography> feature2, R_xlen_t i) {
    S2BooleanOperation::Options options;
    if (model >= 0) {
      options.set_polygon_model(get_polygon_model(model));
      options.set_polyline_model(get_polyline_model(model));
    }
    if (snap_level > 0) { // taking the global variable!!
      options.set_snap_function(s2builderutil::S2CellIdSnapFunction(snap_level));
    }
    return doBooleanOperation<opType>(feature1->ShapeIndex(), feature2->ShapeIndex(), options);
  }
};

// [[Rcpp::export]]
List cpp_s2_intersection(List geog1, List geog2, int model = -1L) {
  List ret;
  if (model == -1 || model == 1) {
    BooleanOperationOp<S2BooleanOperation::OpType::INTERSECTION, 1> op;
    ret = op.processVector(geog1, geog2);
  } else if (model == 0) {
    BooleanOperationOp<S2BooleanOperation::OpType::INTERSECTION, 0> op;
    ret = op.processVector(geog1, geog2);
  } else if (model == 2) {
    BooleanOperationOp<S2BooleanOperation::OpType::INTERSECTION, 2> op;
    ret = op.processVector(geog1, geog2);
  }
  return ret;
}

// [[Rcpp::export]]
List cpp_s2_union(List geog1, List geog2, int model = -1L) {
  List ret;
  if (model == -1 || model == 1) {
    BooleanOperationOp<S2BooleanOperation::OpType::UNION, 1> op;
    ret = op.processVector(geog1, geog2);
  } else if (model == 0) {
    BooleanOperationOp<S2BooleanOperation::OpType::UNION, 0> op;
    ret = op.processVector(geog1, geog2);
  } else if (model == 2) {
    BooleanOperationOp<S2BooleanOperation::OpType::UNION, 2> op;
    ret = op.processVector(geog1, geog2);
  }
  return ret;
}

// [[Rcpp::export]]
List cpp_s2_difference(List geog1, List geog2, int model = -1L) {
  List ret;
  if (model == -1 || model == 1) {
    BooleanOperationOp<S2BooleanOperation::OpType::DIFFERENCE, 1> op;
    ret = op.processVector(geog1, geog2);
  } else if (model == 0) {
    BooleanOperationOp<S2BooleanOperation::OpType::DIFFERENCE, 0> op;
    ret = op.processVector(geog1, geog2);
  } else if (model == 2) {
    BooleanOperationOp<S2BooleanOperation::OpType::DIFFERENCE, 2> op;
    ret = op.processVector(geog1, geog2);
  }
  return ret;
}

// [[Rcpp::export]]
List cpp_s2_symdifference(List geog1, List geog2, int model = -1L) {
  List ret;
  if (model == -1 || model == 1) {
    BooleanOperationOp<S2BooleanOperation::OpType::SYMMETRIC_DIFFERENCE, 1> op;
    ret = op.processVector(geog1, geog2);
  } else if (model == 0) {
    BooleanOperationOp<S2BooleanOperation::OpType::SYMMETRIC_DIFFERENCE, 0> op;
    ret = op.processVector(geog1, geog2);
  } else if (model == 2) {
    BooleanOperationOp<S2BooleanOperation::OpType::SYMMETRIC_DIFFERENCE, 2> op;
    ret = op.processVector(geog1, geog2);
  }
  return ret;
}

// [[Rcpp::export]]
List cpp_s2_union_agg(List geog, bool naRm) {
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
  List output(1);
  MutableS2ShapeIndex emptyIndex;
  output[0] = doBooleanOperation<S2BooleanOperation::OpType::UNION>(&index, &emptyIndex, options);
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
List cpp_s2_closestpoint(List geog1, List geog2) {
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
        S2Polyline *pl = new S2Polyline;
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
List cpp_s2_nearestfeature(List geog1, List geog2) {
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
