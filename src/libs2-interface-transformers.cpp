
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

#include "libs2-snap.h"
#include "libs2-model.h"
#include "libs2-geography-operator.h"
#include "libs2-point-geography.h"
#include "libs2-polyline-geography.h"
#include "libs2-polygon-geography.h"
#include "libs2-geography-collection.h"

#include <Rcpp.h>
using namespace Rcpp;

template <S2BooleanOperation::OpType opType>
Rcpp::XPtr<LibS2Geography> doBooleanOperation(S2ShapeIndex* index1, S2ShapeIndex* index2, 
      S2BooleanOperation::Options options) {

  std::vector<S2Point> points;
  std::vector<std::unique_ptr<S2Polyline>> polylines;
  std::unique_ptr<S2Polygon> polygon = absl::make_unique<S2Polygon>();

  std::vector<std::unique_ptr<S2Builder::Layer>> layers;
  layers.push_back(absl::make_unique<s2builderutil::S2PointVectorLayer>(&points));
  layers.push_back(absl::make_unique<s2builderutil::S2PolylineVectorLayer>(&polylines));
  layers.push_back(absl::make_unique<s2builderutil::S2PolygonLayer>(polygon.get()));

  S2BooleanOperation op(opType, std::move(layers), options);

  FLAGS_s2debug = false;

  S2Error error;
  if (!op.Build(*index1, *index2, &error)) {
    stop(error.text());
  }

  // count non-empty dimensions
  int nonEmptyDimensions = (!polygon->is_empty() + (polylines.size() > 0) + (points.size() > 0));

  if (nonEmptyDimensions == 0 && 
       !S2BooleanOperation::IsEmpty(opType, *index1, *index2, options)) {
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
     if (!op2.Build(*index1, *index2, &error)) {
       stop(error.text());
	 }
	 
	 /*
	 for (int i = 0; i < index.num_shape_ids(); i++) {
       // std::unique_ptr<S2Shape> shp = index.Release(i);
       S2Shape *shp = index.shape(i);
	 }
	 */
	 Rcpp::Rcout << "number of messy features: " << index.num_shape_ids() << std::endl;
	 Rcpp::XPtr<LibS2Geography> feature(new LibS2GeographyCollection());
     feature->BuildShapeIndex(&index);
	 return feature;
   }

  // return empty output
  if (nonEmptyDimensions == 0) {
    if (!S2BooleanOperation::IsEmpty(opType, *index1, *index2, options))
      warning("result is not empty, but we found no vertices!");
    return Rcpp::XPtr<LibS2Geography>(new LibS2GeographyCollection());
  }

  // return mixed output
  if (nonEmptyDimensions > 1) {
    std::vector<std::unique_ptr<LibS2Geography>> features;

    if (points.size() > 0) {
      features.push_back(absl::make_unique<LibS2PointGeography>(std::move(points)));
    }

    if (polylines.size() > 0) {
      features.push_back(absl::make_unique<LibS2PolylineGeography>(std::move(polylines)));
    }

    if (!polygon->is_empty()) {
      features.push_back(absl::make_unique<LibS2PolygonGeography>(std::move(polygon)));
    }

    return Rcpp::XPtr<LibS2Geography>(new LibS2GeographyCollection(std::move(features)));
  }

  // return single dimension output
  if (!polygon->is_empty()) {
    return Rcpp::XPtr<LibS2Geography>(new LibS2PolygonGeography(std::move(polygon)));
  } else if (polylines.size() > 0) {
    return Rcpp::XPtr<LibS2Geography>(new LibS2PolylineGeography(std::move(polylines)));
  } else {
    return Rcpp::XPtr<LibS2Geography>(new LibS2PointGeography(std::move(points)));
  }
}

template <S2BooleanOperation::OpType opType, int model>
class LibS2BooleanOperationOp: public LibS2BinaryGeographyOperator<List, SEXP> {
  SEXP processFeature(XPtr<LibS2Geography> feature1, XPtr<LibS2Geography> feature2, R_xlen_t i) {
    S2BooleanOperation::Options options;
	if (model >= 0) {
      options.set_polygon_model(get_polygon_model(model));
      options.set_polyline_model(get_polyline_model(model));
	}
    return doBooleanOperation<opType>(feature1->ShapeIndex(), feature2->ShapeIndex(), options);
  }
};

// [[Rcpp::export]]
List libs2_cpp_s2_intersection(List geog1, List geog2, int model = -1) {
  if (model == -1 || model == 1) {
    LibS2BooleanOperationOp<S2BooleanOperation::OpType::INTERSECTION, 1> op;
    return op.processVector(geog1, geog2);
  } else if (model == 0) {
    LibS2BooleanOperationOp<S2BooleanOperation::OpType::INTERSECTION, 0> op;
    return op.processVector(geog1, geog2);
  } else if (model == 2) {
    LibS2BooleanOperationOp<S2BooleanOperation::OpType::INTERSECTION, 2> op;
    return op.processVector(geog1, geog2);
  }
}

// [[Rcpp::export]]
List libs2_cpp_s2_union(List geog1, List geog2, int model = -1) {
  if (model == -1 || model == 1) {
    LibS2BooleanOperationOp<S2BooleanOperation::OpType::UNION, 1> op;
    return op.processVector(geog1, geog2);
  } else if (model == 0) {
    LibS2BooleanOperationOp<S2BooleanOperation::OpType::UNION, 0> op;
    return op.processVector(geog1, geog2);
  } else if (model == 2) {
    LibS2BooleanOperationOp<S2BooleanOperation::OpType::UNION, 2> op;
    return op.processVector(geog1, geog2);
  }
}

// [[Rcpp::export]]
List libs2_cpp_s2_difference(List geog1, List geog2, int model = -1) {
  if (model == -1 || model == 1) {
    LibS2BooleanOperationOp<S2BooleanOperation::OpType::DIFFERENCE, 1> op;
    return op.processVector(geog1, geog2);
  } else if (model == 0) {
    LibS2BooleanOperationOp<S2BooleanOperation::OpType::DIFFERENCE, 0> op;
    return op.processVector(geog1, geog2);
  } else if (model == 2) {
    LibS2BooleanOperationOp<S2BooleanOperation::OpType::DIFFERENCE, 2> op;
    return op.processVector(geog1, geog2);
  }
}

// [[Rcpp::export]]
List libs2_cpp_s2_symdifference(List geog1, List geog2, int model = -1) {
  if (model == -1 || model == 1) {
    LibS2BooleanOperationOp<S2BooleanOperation::OpType::SYMMETRIC_DIFFERENCE, 1> op;
    return op.processVector(geog1, geog2);
  } else if (model == 0) {
    LibS2BooleanOperationOp<S2BooleanOperation::OpType::SYMMETRIC_DIFFERENCE, 0> op;
    return op.processVector(geog1, geog2);
  } else if (model == 2) {
    LibS2BooleanOperationOp<S2BooleanOperation::OpType::SYMMETRIC_DIFFERENCE, 2> op;
    return op.processVector(geog1, geog2);
  }
}

// [[Rcpp::export]]
List libs2_cpp_s2_union_agg(List geog, bool naRm) {
  MutableS2ShapeIndex index;

  SEXP item;
  for (R_xlen_t i = 0; i < geog.size(); i++) {
    item = geog[i];
    if (item == R_NilValue && !naRm) {
      return List::create(R_NilValue);
    }

    if (item != R_NilValue) {
      Rcpp::XPtr<LibS2Geography> feature(item);
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
List libs2_cpp_s2_centroid_agg(List geog, bool naRm) {
  S2Point cumCentroid;

  SEXP item;
  for (R_xlen_t i = 0; i < geog.size(); i++) {
    item = geog[i];
    if (item == R_NilValue && !naRm) {
      return List::create(R_NilValue);
    }

    if (item != R_NilValue) {
      Rcpp::XPtr<LibS2Geography> feature(item);
      S2Point centroid = feature->Centroid();
      if (centroid.Norm2() > 0) {
        cumCentroid += centroid.Normalize();
      }
    }
  }

  List output(1);
  if (cumCentroid.Norm2() == 0) {
    output[0] = Rcpp::XPtr<LibS2Geography>(new LibS2PointGeography());
  } else {
    output[0] = Rcpp::XPtr<LibS2Geography>(new LibS2PointGeography(cumCentroid));
  }

  return output;
}

// [[Rcpp::export]]
List libs2_cpp_s2_closestpoint(List geog1, List geog2) {
  class LibS2Op: public LibS2BinaryGeographyOperator<List, SEXP> {

    SEXP processFeature(XPtr<LibS2Geography> feature1, XPtr<LibS2Geography> feature2, R_xlen_t i) {
      S2ClosestEdgeQuery query(feature1->ShapeIndex());
      S2ClosestEdgeQuery::ShapeIndexTarget target(feature2->ShapeIndex());

      const auto& result = query.FindClosestEdge(&target);

      //  result.edge_id() == -1 means there was no match
      if (result.edge_id() == -1) {
        return XPtr<LibS2Geography>(new LibS2PointGeography());
      }

      // get the edge on feature1 that is closest to feature2
      // the point returned here must be somewhere along this edge
      const S2Shape::Edge edge1 = query.GetEdge(result);

      // the edge on feature 1 *is* a point: easy!
      if (edge1.v0 == edge1.v1) {
        return XPtr<LibS2Geography>(new LibS2PointGeography(edge1.v0));
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
        return XPtr<LibS2Geography>(new LibS2PointGeography(closest));
      } else {
        stop("Don't know how to find the closest point given two non-point edges");
      }
    }
  };

  LibS2Op op;
  return op.processVector(geog1, geog2);
}

// [[Rcpp::export]]
List libs2_cpp_s2_centroid(List geog) {
  class LibS2Op: public LibS2UnaryGeographyOperator<List, SEXP> {
    SEXP processFeature(XPtr<LibS2Geography> feature, R_xlen_t i) {
      S2Point centroid = feature->Centroid();
      if (centroid.Norm2() == 0) {
        return XPtr<LibS2Geography>(new LibS2PointGeography());
      } else {
        return XPtr<LibS2Geography>(new LibS2PointGeography(centroid.Normalize()));
      }
    }
  };

  LibS2Op op;
  return op.processVector(geog);
}

// [[Rcpp::export]]
List libs2_cpp_s2_boundary(List geog) {
  class LibS2Op: public LibS2UnaryGeographyOperator<List, SEXP> {
    SEXP processFeature(XPtr<LibS2Geography> feature, R_xlen_t i) {
      std::unique_ptr<LibS2Geography> ptr = feature->Boundary();
      return XPtr<LibS2Geography>(ptr.release());
    }
  };

  LibS2Op op;
  return op.processVector(geog);
}
