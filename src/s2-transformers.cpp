
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
                                               std::unique_ptr<S2Polygon> polygon,
                                               int dimensions) {
  // count non-empty dimensions
  bool has_polygon = (dimensions & GeographyOperationOptions::Dimension::POLYGON) &&
    !polygon->is_empty();
  bool has_polyline = (dimensions & GeographyOperationOptions::Dimension::POLYLINE) &&
    (polylines.size() > 0);
  bool has_points = (dimensions & GeographyOperationOptions::Dimension::POINT) &&
    (points.size() > 0);
  int nonEmptyDimensions = has_polygon + has_polyline + has_points;

  // return empty output
  if (nonEmptyDimensions == 0) {
    return absl::make_unique<GeographyCollection>();
  }

  // return mixed dimension output
  if (nonEmptyDimensions > 1) {
    std::vector<std::unique_ptr<Geography>> features;

    if (has_points) {
      features.push_back(absl::make_unique<PointGeography>(std::move(points)));
    }

    if (has_polyline) {
      features.push_back(absl::make_unique<PolylineGeography>(std::move(polylines)));
    }

    if (has_polygon) {
      features.push_back(absl::make_unique<PolygonGeography>(std::move(polygon)));
    }

    return absl::make_unique<GeographyCollection>(std::move(features));
  }

  // return single dimension output
  if (has_polygon) {
    return absl::make_unique<PolygonGeography>(std::move(polygon));
  } else if (has_polyline) {
    return absl::make_unique<PolylineGeography>(std::move(polylines));
  } else {
    return absl::make_unique<PointGeography>(std::move(points));
  }
}

std::unique_ptr<Geography> doBooleanOperation(S2ShapeIndex* index1, S2ShapeIndex* index2,
                                              S2BooleanOperation::OpType opType,
                                              S2BooleanOperation::Options options,
                                              GeographyOperationOptions::LayerOptions layerOptions) {

  // create the data structures that will contain the output
  std::vector<S2Point> points;
  std::vector<std::unique_ptr<S2Polyline>> polylines;
  std::unique_ptr<S2Polygon> polygon = absl::make_unique<S2Polygon>();

  s2builderutil::LayerVector layers(3);
  layers[0] = absl::make_unique<s2builderutil::S2PointVectorLayer>(&points, layerOptions.pointLayerOptions);
  layers[1] = absl::make_unique<s2builderutil::S2PolylineVectorLayer>(&polylines, layerOptions.polylineLayerOptions);
  layers[2] = absl::make_unique<s2builderutil::S2PolygonLayer>(polygon.get(), layerOptions.polygonLayerOptions);

  // do the boolean operation
  S2BooleanOperation booleanOp(
    opType,
    // normalizing the closed set here is required for line intersections
    // to work as expected
    s2builderutil::NormalizeClosedSet(std::move(layers)),
    options
  );

  // build and check for errors
  S2Error error;
  if (!booleanOp.Build(*index1, *index2, &error)) {
    stop(error.text());
  }

  // construct output
  return geographyFromLayers(
    std::move(points),
    std::move(polylines),
    std::move(polygon),
    layerOptions.dimensions
  );
}

std::unique_ptr<Geography> rebuildGeography(S2ShapeIndex* index,
                                            S2Builder::Options options,
                                            GeographyOperationOptions::LayerOptions layerOptions) {
  // create the builder
  S2Builder builder(options);

  // create the data structures that will contain the output
  std::vector<S2Point> points;
  std::vector<std::unique_ptr<S2Polyline>> polylines;
  std::unique_ptr<S2Polygon> polygon = absl::make_unique<S2Polygon>();

  // add shapes to the layer with the appropriate dimension
  builder.StartLayer(
    absl::make_unique<s2builderutil::S2PointVectorLayer>(&points, layerOptions.pointLayerOptions)
  );
  for (S2Shape* shape : *index) {
    if (shape->dimension() == 0) {
      builder.AddShape(*shape);
    }
  }

  builder.StartLayer(
    absl::make_unique<s2builderutil::S2PolylineVectorLayer>(&polylines, layerOptions.polylineLayerOptions)
  );
  for (S2Shape* shape : *index) {
    if (shape->dimension() == 1) {
      builder.AddShape(*shape);
    }
  }

  builder.StartLayer(
    absl::make_unique<s2builderutil::S2PolygonLayer>(polygon.get(), layerOptions.polygonLayerOptions)
  );
  for (S2Shape* shape : *index) {
    if (shape->dimension() == 2) {
      builder.AddShape(*shape);
    }
  }

  // build the output
  S2Error error;
  if (!builder.Build(&error)) {
    throw GeographyOperatorException(error.text());
  }

  // construct output
  return geographyFromLayers(
    std::move(points),
    std::move(polylines),
    std::move(polygon),
    layerOptions.dimensions
  );
}

class BooleanOperationOp: public BinaryGeographyOperator<List, SEXP> {
public:
  BooleanOperationOp(S2BooleanOperation::OpType opType, List s2options):
    opType(opType) {
      GeographyOperationOptions options(s2options);
      this->options = options.booleanOperationOptions();
      this->layerOptions = options.layerOptions();
    }

  SEXP processFeature(XPtr<Geography> feature1, XPtr<Geography> feature2, R_xlen_t i) {
    std::unique_ptr<Geography> geography = doBooleanOperation(
      feature1->ShapeIndex(),
      feature2->ShapeIndex(),
      this->opType,
      this->options,
      this->layerOptions
    );

    return Rcpp::XPtr<Geography>(geography.release());
  }

private:
  S2BooleanOperation::OpType opType;
  S2BooleanOperation::Options options;
  GeographyOperationOptions::LayerOptions layerOptions;
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
List cpp_s2_coverage_union_agg(List geog, List s2options, bool naRm) {
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
    options.booleanOperationOptions(),
    options.layerOptions()
  );

  return List::create(Rcpp::XPtr<Geography>(geography.release()));
}

// This approach to aggregation is slow but accurate. There is probably a more efficient way
// to accumulate geometries and/or re-use the layers vector but thus far I haven't figured
// out a way to make that work.
// [[Rcpp::export]]
List cpp_s2_union_agg(List geog, List s2options, bool naRm) {
  GeographyOperationOptions options(s2options);
  GeographyOperationOptions::LayerOptions layerOptions = options.layerOptions();
  S2BooleanOperation::Options unionOptions = options.booleanOperationOptions();
  S2Builder::Options buillderOptions = options.builderOptions();

  // using smart pointers here so that we can use swap() to
  // use replace accumulatedIndex with index after each union
  std::unique_ptr<MutableS2ShapeIndex> index = absl::make_unique<MutableS2ShapeIndex>();
  std::unique_ptr<MutableS2ShapeIndex> accumulatedIndex = absl::make_unique<MutableS2ShapeIndex>();

  SEXP item;
  for (R_xlen_t i = 0; i < geog.size(); i++) {
    item = geog[i];
    if (item == R_NilValue && !naRm) {
      return List::create(R_NilValue);
    }

    if (item != R_NilValue) {
      Rcpp::XPtr<Geography> feature(item);

      index->Clear();
      s2builderutil::LayerVector layers(3);
      layers[0] = absl::make_unique<s2builderutil::IndexedS2PointVectorLayer>(index.get(), layerOptions.pointLayerOptions);
      layers[1] = absl::make_unique<s2builderutil::IndexedS2PolylineVectorLayer>(index.get(), layerOptions.polylineLayerOptions);
      layers[2] = absl::make_unique<s2builderutil::IndexedS2PolygonLayer>(index.get(), layerOptions.polygonLayerOptions);

      S2BooleanOperation booleanOp(
        S2BooleanOperation::OpType::UNION,
        s2builderutil::NormalizeClosedSet(std::move(layers)),
        unionOptions
      );

      S2Error error;
      if (!booleanOp.Build(*accumulatedIndex, *(feature->ShapeIndex()), &error)) {
        stop(error.text());
      }

      accumulatedIndex.swap(index);
    }
  }

  std::unique_ptr<Geography> geography = rebuildGeography(
    accumulatedIndex.get(),
    options.builderOptions(),
    options.layerOptions()
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
    output[0] = Rcpp::XPtr<Geography>(new PointGeography(cumCentroid.Normalize()));
  }

  return output;
}

// [[Rcpp::export]]
List cpp_s2_rebuild_agg(List geog, List s2options, bool naRm) {
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

  std::unique_ptr<Geography> geography = rebuildGeography(
    &index,
    options.builderOptions(),
    options.layerOptions()
  );

  return List::create(Rcpp::XPtr<Geography>(geography.release()));
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
List cpp_s2_rebuild(List geog, List s2options) {
  class Op: public UnaryGeographyOperator<List, SEXP> {
  public:
    Op(List s2options) {
      GeographyOperationOptions options(s2options);
      this->options = options.builderOptions();
      this->layerOptions = options.layerOptions();
    }

    SEXP processFeature(XPtr<Geography> feature, R_xlen_t i) {
      std::unique_ptr<Geography> ptr = rebuildGeography(
        feature->ShapeIndex(),
        this->options,
        this->layerOptions
      );
      return XPtr<Geography>(ptr.release());
    }

  private:
    S2Builder::Options options;
    GeographyOperationOptions::LayerOptions layerOptions;
  };

  Op op(s2options);
  return op.processVector(geog);
}

// [[Rcpp::export]]
List cpp_s2_unary_union(List geog, List s2options) {
  class Op: public UnaryGeographyOperator<List, SEXP> {
  public:
    Op(List s2options) {
      GeographyOperationOptions options(s2options);
      this->options = options.booleanOperationOptions();
      this->layerOptions = options.layerOptions();
    }

    SEXP processFeature(XPtr<Geography> feature, R_xlen_t i) {
      // complex union only needed when a polygon is involved
      bool simpleUnionOK = feature->IsEmpty() ||
        (feature->Dimension() < 2);

      // valid polygons that are not part of a collection can also use a
      // simple union (common)
      if (feature->GeographyType() == Geography::Type::GEOGRAPHY_POLYGON) {
        S2Error validationError;
        if(!(feature->Polygon()->FindValidationError(&validationError))) {
          simpleUnionOK = true;
        }
      }

      if (simpleUnionOK) {
        MutableS2ShapeIndex emptyIndex;

        std::unique_ptr<Geography> ptr = doBooleanOperation(
          feature->ShapeIndex(),
          &emptyIndex,
          S2BooleanOperation::OpType::UNION,
          this->options,
          this->layerOptions
        );

        return XPtr<Geography>(ptr.release());
      } else if (feature->GeographyType() == Geography::Type::GEOGRAPHY_POLYGON) {
        // If we've made it here we have an invalid polygon on our hands. A geography with
        // invalid loops won't work with the S2BooleanOperation we will use to accumulate
        // (i.e., union) valid polygons, so we need to rebuild each loop as its own polygon,
        // splitting crossed edges along the way.
        const S2Polygon* originalPoly = feature->Polygon();

        // Not exposing these options as an argument (except snap function)
        // because a particular combiation of them is required for this to work
        S2Builder::Options builderOptions;
        builderOptions.set_split_crossing_edges(true);
        builderOptions.set_snap_function(this->options.snap_function());
        s2builderutil::S2PolygonLayer::Options layerOptions;
        layerOptions.set_edge_type(S2Builder::EdgeType::UNDIRECTED);
        layerOptions.set_validate(false);

        // Rebuild all loops as polygons using the S2Builder()
        std::vector<std::unique_ptr<S2Polygon>> loops;
        for (int i = 0; i < originalPoly->num_loops(); i++) {
          std::unique_ptr<S2Polygon> loop = absl::make_unique<S2Polygon>();
          S2Builder builder(builderOptions);
          builder.StartLayer(absl::make_unique<s2builderutil::S2PolygonLayer>(loop.get()));
          builder.AddShape(S2Loop::Shape(originalPoly->loop(i)));
          S2Error error;
          if (!builder.Build(&error)) {
            throw GeographyOperatorException(error.text());
          }

          // Check if the builder created a polygon whose boundary contained more than
          // half the earth (and invert it if so)
          if (loop->GetArea() > (2 * M_PI)) {
            loop->Invert();
          }

          loops.push_back(std::move(loop));
        }

        // Accumulate the union of outer loops (but difference of inner loops)
        std::unique_ptr<S2Polygon> accumulatedPolygon = absl::make_unique<S2Polygon>();
        for (int i = 0; i < originalPoly->num_loops(); i++) {
          std::unique_ptr<S2Polygon> polygonResult = absl::make_unique<S2Polygon>();

          // Use original nesting to suggest if this loop should be unioned or diffed.
          // For valid polygons loops are arranged such that the biggest loop is on the outside
          // followed by holes such that the below strategy should work (since we are
          // just iterating along the original loop structure)
          if ((originalPoly->loop(i)->depth() % 2) == 0) {
            polygonResult->InitToUnion(accumulatedPolygon.get(), loops[i].get());
          } else {
            polygonResult->InitToDifference(accumulatedPolygon.get(), loops[i].get());
          }

          accumulatedPolygon.swap(polygonResult);
        }

        return XPtr<Geography>(new PolygonGeography(std::move(accumulatedPolygon)));
      } else {
        // This is a less common case (mixed dimension output that includes a polygon).
        // In the absence of a clean solution, saving this battle for another day.
        throw GeographyOperatorException("Unary union for collections is not implemented");
      }
    }

  private:
    S2BooleanOperation::Options options;
    GeographyOperationOptions::LayerOptions layerOptions;
  };

  Op op(s2options);
  return op.processVector(geog);
}

// [[Rcpp::export]]
List cpp_s2_interpolate_normalized(List geog, NumericVector distanceNormalized) {
  class Op: public UnaryGeographyOperator<List, SEXP> {
  public:
    NumericVector distanceNormalized;
    Op(NumericVector distanceNormalized): distanceNormalized(distanceNormalized) {}
    SEXP processFeature(XPtr<Geography> feature, R_xlen_t i) {
      if (NumericVector::is_na(this->distanceNormalized[i])) {
        return R_NilValue;
      }

      if (feature->IsCollection()) {
        throw GeographyOperatorException("`x` must be a simple geography");
      }

      if (feature->IsEmpty()) {
        return R_NilValue;
      }

      if (feature->GeographyType() == Geography::Type::GEOGRAPHY_POLYLINE) {
        S2Point point = feature->Polyline()->at(0)->Interpolate(this->distanceNormalized[i]);
        return XPtr<PointGeography>(new PointGeography(point));
      } else {
        throw GeographyOperatorException("`x` must be a polyline geography");
      }
    }
  };

  Op op(distanceNormalized);
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
