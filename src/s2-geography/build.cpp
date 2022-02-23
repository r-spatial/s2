
#include "s2/s2boolean_operation.h"
#include "s2/s2builder.h"
#include "s2/s2builderutil_s2polygon_layer.h"
#include "s2/s2builderutil_s2polyline_vector_layer.h"
#include "s2/s2builderutil_s2point_vector_layer.h"
#include "s2/s2builderutil_closed_set_normalizer.h"

#include "geography.hpp"
#include "build.hpp"

namespace s2geography {

std::unique_ptr<S2Geography> s2_geography_from_layers(std::vector<S2Point> points,
                                                      std::vector<std::unique_ptr<S2Polyline>> polylines,
                                                      std::unique_ptr<S2Polygon> polygon,
                                                      const S2GeographyOptions& options) {
  // count non-empty dimensions
  bool has_polygon = !polygon->is_empty();
  bool has_polylines = polylines.size() > 0;
  bool has_points = points.size() > 0;

  // use the requstested dimensions to produce the right kind of EMTPY
  bool include_polygon = options.polygon_layer_action == S2GeographyOptions::OUTPUT_ACTION_INCLUDE;
  bool include_polylines = options.polyline_layer_action == S2GeographyOptions::OUTPUT_ACTION_INCLUDE;
  bool include_points = options.point_layer_action == S2GeographyOptions::OUTPUT_ACTION_INCLUDE;

  if (has_polygon && options.polygon_layer_action == S2GeographyOptions::OUTPUT_ACTION_ERROR) {
      throw S2GeographyException("Output contained unexpected polygon");
  } else if (has_polygon && options.polygon_layer_action == S2GeographyOptions::OUTPUT_ACTION_IGNORE) {
      has_polygon = false;
  }

  if (has_polylines && options.polyline_layer_action == S2GeographyOptions::OUTPUT_ACTION_ERROR) {
      throw S2GeographyException("Output contained unexpected polylines");
  } else if (has_polylines && options.polyline_layer_action == S2GeographyOptions::OUTPUT_ACTION_IGNORE) {
      has_polylines = false;
  }

  if (has_points && options.point_layer_action == S2GeographyOptions::OUTPUT_ACTION_ERROR) {
      throw S2GeographyException("Output contained unexpected points");
  } else if (has_points && options.point_layer_action == S2GeographyOptions::OUTPUT_ACTION_IGNORE) {
      has_points = false;
  }

  int non_empty_dimensions = has_polygon + has_polylines + has_points;
  int included_dimensions = include_polygon + include_polylines + include_points;

  // return mixed dimension output
  if (non_empty_dimensions > 1) {
    std::vector<std::unique_ptr<S2Geography>> features;

    if (has_points) {
      features.push_back(absl::make_unique<S2GeographyOwningPoint>(std::move(points)));
    }

    if (has_polylines) {
      features.push_back(absl::make_unique<S2GeographyOwningPolyline>(std::move(polylines)));
    }

    if (has_polygon) {
      features.push_back(absl::make_unique<S2GeographyOwningPolygon>(std::move(polygon)));
    }

    return absl::make_unique<S2GeographyCollection>(std::move(features));
  }

  // return single dimension output
  if (has_polygon || (included_dimensions == 1 && include_polygon)) {
    return absl::make_unique<S2GeographyOwningPolygon>(std::move(polygon));
  } else if (has_polylines || (included_dimensions == 1 && include_polylines)) {
    return absl::make_unique<S2GeographyOwningPolyline>(std::move(polylines));
  } else if (has_points || (included_dimensions == 1 && include_points)) {
    return absl::make_unique<S2GeographyOwningPoint>(std::move(points));
  } else {
    return absl::make_unique<S2GeographyCollection>();
  }
}

std::unique_ptr<S2Geography> s2_boolean_operation(const S2GeographyShapeIndex& geog1,
                                                  const S2GeographyShapeIndex& geog2,
                                                  S2BooleanOperation::OpType op_type,
                                                  const S2GeographyOptions& options) {

  // Create the data structures that will contain the output.
  std::vector<S2Point> points;
  std::vector<std::unique_ptr<S2Polyline>> polylines;
  std::unique_ptr<S2Polygon> polygon = absl::make_unique<S2Polygon>();

  s2builderutil::LayerVector layers(3);
  layers[0] = absl::make_unique<s2builderutil::S2PointVectorLayer>(&points, options.point_layer);
  layers[1] = absl::make_unique<s2builderutil::S2PolylineVectorLayer>(&polylines, options.polyline_layer);
  layers[2] = absl::make_unique<s2builderutil::S2PolygonLayer>(polygon.get(), options.polygon_layer);

  // specify the boolean operation
  S2BooleanOperation op(
    op_type,
    // Normalizing the closed set here is required for line intersections
    // to work in the same way as GEOS
    s2builderutil::NormalizeClosedSet(std::move(layers)),
    options.boolean_operation
  );

  // do the boolean operation, build layers, and check for errors
  S2Error error;
  if (!op.Build(geog1.ShapeIndex(), geog2.ShapeIndex(), &error)) {
    throw S2GeographyException(error.text());
  }

  // construct output
  return s2_geography_from_layers(
    std::move(points),
    std::move(polylines),
    std::move(polygon),
    options
  );
}

std::unique_ptr<S2Geography> s2_intersection(const S2GeographyShapeIndex& geog1,
                                             const S2GeographyShapeIndex& geog2,
                                             const S2GeographyOptions& options) {
  return s2_boolean_operation(
    geog1, geog2,
    S2BooleanOperation::OpType::INTERSECTION,
    options);
}

std::unique_ptr<S2Geography> s2_union(const S2GeographyShapeIndex& geog1,
                                      const S2GeographyShapeIndex& geog2,
                                      const S2GeographyOptions& options) {
  return s2_boolean_operation(
    geog1, geog2,
    S2BooleanOperation::OpType::UNION,
    options);
}

std::unique_ptr<S2Geography> s2_difference(const S2GeographyShapeIndex& geog1,
                                           const S2GeographyShapeIndex& geog2,
                                           const S2GeographyOptions& options) {
  return s2_boolean_operation(
    geog1, geog2,
    S2BooleanOperation::OpType::DIFFERENCE,
    options);
}

std::unique_ptr<S2Geography> s2_symmetric_difference(const S2GeographyShapeIndex& geog1,
                                                     const S2GeographyShapeIndex& geog2,
                                                     const S2GeographyOptions& options) {
  return s2_boolean_operation(
    geog1, geog2,
    S2BooleanOperation::OpType::SYMMETRIC_DIFFERENCE,
    options);
}

std::unique_ptr<S2Geography> s2_rebuild(const S2GeographyShapeIndex& geog,
                                        const S2GeographyOptions& options) {
  // create the builder
  S2Builder builder(options.builder);

  // create the data structures that will contain the output
  std::vector<S2Point> points;
  std::vector<std::unique_ptr<S2Polyline>> polylines;
  std::unique_ptr<S2Polygon> polygon = absl::make_unique<S2Polygon>();

  // add shapes to the layer with the appropriate dimension
  builder.StartLayer(
    absl::make_unique<s2builderutil::S2PointVectorLayer>(&points, options.point_layer)
  );
  for (S2Shape* shape : geog.ShapeIndex()) {
    if (shape->dimension() == 0) {
      builder.AddShape(*shape);
    }
  }

  builder.StartLayer(
    absl::make_unique<s2builderutil::S2PolylineVectorLayer>(&polylines, options.polyline_layer)
  );
  for (S2Shape* shape : geog.ShapeIndex()) {
    if (shape->dimension() == 1) {
      builder.AddShape(*shape);
    }
  }

  builder.StartLayer(
    absl::make_unique<s2builderutil::S2PolygonLayer>(polygon.get(), options.polygon_layer)
  );
  for (S2Shape* shape : geog.ShapeIndex()) {
    if (shape->dimension() == 2) {
      builder.AddShape(*shape);
    }
  }

  // build the output
  S2Error error;
  if (!builder.Build(&error)) {
    throw S2GeographyException(error.text());
  }

  // construct output
  return s2_geography_from_layers(
    std::move(points),
    std::move(polylines),
    std::move(polygon),
    options
  );
}

}
