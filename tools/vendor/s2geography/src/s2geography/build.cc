
#include "s2geography/build.h"

#include <s2/s2boolean_operation.h>
#include <s2/s2builder.h>
#include <s2/s2builderutil_closed_set_normalizer.h>
#include <s2/s2builderutil_s2point_vector_layer.h>
#include <s2/s2builderutil_s2polygon_layer.h>
#include <s2/s2builderutil_s2polyline_vector_layer.h>

#include "s2geography/accessors.h"
#include "s2geography/geography.h"

namespace s2geography {

std::unique_ptr<Geography> s2_geography_from_layers(
    std::vector<S2Point> points,
    std::vector<std::unique_ptr<S2Polyline>> polylines,
    std::unique_ptr<S2Polygon> polygon,
    GlobalOptions::OutputAction point_layer_action,
    GlobalOptions::OutputAction polyline_layer_action,
    GlobalOptions::OutputAction polygon_layer_action) {
  // count non-empty dimensions
  bool has_polygon = !polygon->is_empty();
  bool has_polylines = polylines.size() > 0;
  bool has_points = points.size() > 0;

  // use the requstested dimensions to produce the right kind of EMTPY
  bool include_polygon =
      polygon_layer_action == GlobalOptions::OUTPUT_ACTION_INCLUDE;
  bool include_polylines =
      polyline_layer_action == GlobalOptions::OUTPUT_ACTION_INCLUDE;
  bool include_points =
      point_layer_action == GlobalOptions::OUTPUT_ACTION_INCLUDE;

  if (has_polygon &&
      polygon_layer_action == GlobalOptions::OUTPUT_ACTION_ERROR) {
    throw Exception("Output contained unexpected polygon");
  } else if (has_polygon &&
             polygon_layer_action == GlobalOptions::OUTPUT_ACTION_IGNORE) {
    has_polygon = false;
  }

  if (has_polylines &&
      polyline_layer_action == GlobalOptions::OUTPUT_ACTION_ERROR) {
    throw Exception("Output contained unexpected polylines");
  } else if (has_polylines &&
             polyline_layer_action == GlobalOptions::OUTPUT_ACTION_IGNORE) {
    has_polylines = false;
  }

  if (has_points && point_layer_action == GlobalOptions::OUTPUT_ACTION_ERROR) {
    throw Exception("Output contained unexpected points");
  } else if (has_points &&
             point_layer_action == GlobalOptions::OUTPUT_ACTION_IGNORE) {
    has_points = false;
  }

  int non_empty_dimensions = has_polygon + has_polylines + has_points;
  int included_dimensions =
      include_polygon + include_polylines + include_points;

  // return mixed dimension output
  if (non_empty_dimensions > 1) {
    std::vector<std::unique_ptr<Geography>> features;

    if (has_points) {
      features.push_back(absl::make_unique<PointGeography>(std::move(points)));
    }

    if (has_polylines) {
      features.push_back(
          absl::make_unique<PolylineGeography>(std::move(polylines)));
    }

    if (has_polygon) {
      features.push_back(
          absl::make_unique<PolygonGeography>(std::move(polygon)));
    }

    return absl::make_unique<GeographyCollection>(std::move(features));
  }

  // return single dimension output
  if (has_polygon || (included_dimensions == 1 && include_polygon)) {
    return absl::make_unique<PolygonGeography>(std::move(polygon));
  } else if (has_polylines || (included_dimensions == 1 && include_polylines)) {
    return absl::make_unique<PolylineGeography>(std::move(polylines));
  } else if (has_points || (included_dimensions == 1 && include_points)) {
    return absl::make_unique<PointGeography>(std::move(points));
  } else {
    return absl::make_unique<GeographyCollection>();
  }
}

std::unique_ptr<Geography> s2_boolean_operation(
    const ShapeIndexGeography& geog1, const ShapeIndexGeography& geog2,
    S2BooleanOperation::OpType op_type, const GlobalOptions& options) {
  // Create the data structures that will contain the output.
  std::vector<S2Point> points;
  std::vector<std::unique_ptr<S2Polyline>> polylines;
  std::unique_ptr<S2Polygon> polygon = absl::make_unique<S2Polygon>();

  s2builderutil::LayerVector layers(3);
  layers[0] = absl::make_unique<s2builderutil::S2PointVectorLayer>(
      &points, options.point_layer);
  layers[1] = absl::make_unique<s2builderutil::S2PolylineVectorLayer>(
      &polylines, options.polyline_layer);
  layers[2] = absl::make_unique<s2builderutil::S2PolygonLayer>(
      polygon.get(), options.polygon_layer);

  // specify the boolean operation
  S2BooleanOperation op(op_type,
                        // Normalizing the closed set here is required for line
                        // intersections to work in the same way as GEOS
                        s2builderutil::NormalizeClosedSet(std::move(layers)),
                        options.boolean_operation);

  // do the boolean operation, build layers, and check for errors
  S2Error error;
  if (!op.Build(geog1.ShapeIndex(), geog2.ShapeIndex(), &error)) {
    throw Exception(error.text());
  }

  // construct output
  return s2_geography_from_layers(
      std::move(points), std::move(polylines), std::move(polygon),
      options.point_layer_action, options.polyline_layer_action,
      options.polygon_layer_action);
}

std::unique_ptr<PolygonGeography> s2_unary_union(const PolygonGeography& geog,
                                                 const GlobalOptions& options) {
  // A geography with invalid loops won't work with the S2BooleanOperation
  // we will use to accumulate (i.e., union) valid polygons,
  // so we need to rebuild each loop as its own polygon,
  // splitting crossed edges along the way.

  // Not exposing these options as an argument (except snap function)
  // because a particular combiation of them is required for this to work
  S2Builder::Options builder_options;
  builder_options.set_split_crossing_edges(true);
  builder_options.set_snap_function(options.boolean_operation.snap_function());
  s2builderutil::S2PolygonLayer::Options layer_options;
  layer_options.set_edge_type(S2Builder::EdgeType::UNDIRECTED);
  layer_options.set_validate(false);

  // Rebuild all loops as polygons using the S2Builder()
  std::vector<std::unique_ptr<S2Polygon>> loops;
  for (int i = 0; i < geog.Polygon()->num_loops(); i++) {
    std::unique_ptr<S2Polygon> loop = absl::make_unique<S2Polygon>();
    S2Builder builder(builder_options);
    builder.StartLayer(absl::make_unique<s2builderutil::S2PolygonLayer>(
        loop.get(), layer_options));
    builder.AddShape(S2Loop::Shape(geog.Polygon()->loop(i)));
    S2Error error;
    if (!builder.Build(&error)) {
      throw Exception(error.text());
    }

    // Check if the builder created a polygon whose boundary contained more than
    // half the earth (and invert it if so)
    if (loop->GetArea() > (2 * M_PI)) {
      loop->Invert();
    }

    loops.push_back(std::move(loop));
  }

  // Accumulate the union of outer loops (but difference of inner loops)
  std::unique_ptr<S2Polygon> accumulated_polygon =
      absl::make_unique<S2Polygon>();
  for (int i = 0; i < geog.Polygon()->num_loops(); i++) {
    std::unique_ptr<S2Polygon> polygon_result = absl::make_unique<S2Polygon>();

    // Use original nesting to suggest if this loop should be unioned or diffed.
    // For valid polygons loops are arranged such that the biggest loop is on
    // the outside followed by holes such that the below strategy should work
    // (since we are just iterating along the original loop structure)
    if ((geog.Polygon()->loop(i)->depth() % 2) == 0) {
      polygon_result->InitToUnion(*accumulated_polygon, *loops[i]);
    } else {
      polygon_result->InitToDifference(*accumulated_polygon, *loops[i]);
    }

    accumulated_polygon.swap(polygon_result);
  }

  return absl::make_unique<PolygonGeography>(std::move(accumulated_polygon));
}

std::unique_ptr<Geography> s2_unary_union(const ShapeIndexGeography& geog,
                                          const GlobalOptions& options) {
  // complex union only needed when a polygon is involved
  bool simple_union_ok = s2_is_empty(geog) || s2_dimension(geog) < 2;

  // valid polygons that are not part of a collection can also use a
  // simple union (common)
  if (geog.dimension() == 2) {
    S2Error validation_error;
    if (!s2_find_validation_error(geog, &validation_error)) {
      simple_union_ok = true;
    }
  }

  if (simple_union_ok) {
    ShapeIndexGeography empty;
    return s2_boolean_operation(geog, empty, S2BooleanOperation::OpType::UNION,
                                options);
  }

  if (geog.dimension() == 2) {
    // If we've made it here we have an invalid polygon on our hands.
    auto poly_ptr = dynamic_cast<const PolygonGeography*>(&geog);
    if (poly_ptr != nullptr) {
      return s2_unary_union(*poly_ptr, options);
    } else {
      auto poly = s2_build_polygon(geog);
      return s2_unary_union(*poly, options);
    }
  }

  throw Exception(
      "s2_unary_union() for multidimensional collections not implemented");
}

std::unique_ptr<Geography> s2_rebuild(
    const Geography& geog, const GlobalOptions& options,
    GlobalOptions::OutputAction /*point_layer_action*/,
    GlobalOptions::OutputAction /*polyline_layer_action*/,
    GlobalOptions::OutputAction /*polygon_layer_action*/) {
  // create the builder
  S2Builder builder(options.builder);

  // create the data structures that will contain the output
  std::vector<S2Point> points;
  std::vector<std::unique_ptr<S2Polyline>> polylines;
  std::unique_ptr<S2Polygon> polygon = absl::make_unique<S2Polygon>();

  // add shapes to the layer with the appropriate dimension
  builder.StartLayer(absl::make_unique<s2builderutil::S2PointVectorLayer>(
      &points, options.point_layer));
  for (int i = 0; i < geog.num_shapes(); i++) {
    auto shape = geog.Shape(i);
    if (shape->dimension() == 0) {
      builder.AddShape(*shape);
    }
  }

  builder.StartLayer(absl::make_unique<s2builderutil::S2PolylineVectorLayer>(
      &polylines, options.polyline_layer));
  for (int i = 0; i < geog.num_shapes(); i++) {
    auto shape = geog.Shape(i);
    if (shape->dimension() == 1) {
      builder.AddShape(*shape);
    }
  }

  builder.StartLayer(absl::make_unique<s2builderutil::S2PolygonLayer>(
      polygon.get(), options.polygon_layer));
  for (int i = 0; i < geog.num_shapes(); i++) {
    auto shape = geog.Shape(i);
    if (shape->dimension() == 2) {
      builder.AddShape(*shape);
    }
  }

  // build the output
  S2Error error;
  if (!builder.Build(&error)) {
    throw Exception(error.text());
  }

  // construct output
  return s2_geography_from_layers(
      std::move(points), std::move(polylines), std::move(polygon),
      options.point_layer_action, options.polyline_layer_action,
      options.polygon_layer_action);
}

std::unique_ptr<Geography> s2_rebuild(const Geography& geog,
                                      const GlobalOptions& options) {
  return s2_rebuild(geog, options, options.point_layer_action,
                    options.polyline_layer_action,
                    options.polygon_layer_action);
}

std::unique_ptr<PointGeography> s2_build_point(const Geography& geog) {
  std::unique_ptr<Geography> geog_out = s2_rebuild(
      geog, GlobalOptions(), GlobalOptions::OutputAction::OUTPUT_ACTION_INCLUDE,
      GlobalOptions::OutputAction::OUTPUT_ACTION_ERROR,
      GlobalOptions::OutputAction::OUTPUT_ACTION_ERROR);

  return std::unique_ptr<PointGeography>(
      dynamic_cast<PointGeography*>(geog_out.release()));
}

std::unique_ptr<PolylineGeography> s2_build_polyline(const Geography& geog) {
  std::unique_ptr<Geography> geog_out = s2_rebuild(
      geog, GlobalOptions(), GlobalOptions::OutputAction::OUTPUT_ACTION_ERROR,
      GlobalOptions::OutputAction::OUTPUT_ACTION_INCLUDE,
      GlobalOptions::OutputAction::OUTPUT_ACTION_ERROR);

  return std::unique_ptr<PolylineGeography>(
      dynamic_cast<PolylineGeography*>(geog_out.release()));
}

std::unique_ptr<PolygonGeography> s2_build_polygon(const Geography& geog) {
  std::unique_ptr<Geography> geog_out = s2_rebuild(
      geog, GlobalOptions(), GlobalOptions::OutputAction::OUTPUT_ACTION_ERROR,
      GlobalOptions::OutputAction::OUTPUT_ACTION_ERROR,
      GlobalOptions::OutputAction::OUTPUT_ACTION_INCLUDE);

  return std::unique_ptr<PolygonGeography>(
      dynamic_cast<PolygonGeography*>(geog_out.release()));
}

void RebuildAggregator::Add(const Geography& geog) { index_.Add(geog); }

std::unique_ptr<Geography> RebuildAggregator::Finalize() {
  return s2_rebuild(index_, options_);
}

void S2CoverageUnionAggregator::Add(const Geography& geog) { index_.Add(geog); }

std::unique_ptr<Geography> S2CoverageUnionAggregator::Finalize() {
  ShapeIndexGeography empty_index_;
  return s2_boolean_operation(index_, empty_index_,
                              S2BooleanOperation::OpType::UNION, options_);
}

void S2UnionAggregator::Add(const Geography& geog) {
  if (geog.dimension() == 0 || geog.dimension() == 1) {
    root_.index1.Add(geog);
    return;
  }

  if (other_.size() == 0) {
    other_.push_back(absl::make_unique<Node>());
    other_.back()->index1.Add(geog);
    return;
  }

  Node* last = other_.back().get();
  if (last->index1.num_shapes() == 0) {
    last->index1.Add(geog);
  } else if (last->index2.num_shapes() == 0) {
    last->index2.Add(geog);
  } else {
    other_.push_back(absl::make_unique<Node>());
    other_.back()->index1.Add(geog);
  }
}

std::unique_ptr<Geography> S2UnionAggregator::Node::Merge(
    const GlobalOptions& options) {
  return s2_boolean_operation(index1, index2, S2BooleanOperation::OpType::UNION,
                              options);
}

std::unique_ptr<Geography> S2UnionAggregator::Finalize() {
  for (int j = 0; j < 100; j++) {
    if (other_.size() <= 1) {
      break;
    }

    for (int64_t i = static_cast<int64_t>(other_.size()) - 1; i >= 1;
         i = i - 2) {
      // merge other_[i] with other_[i - 1]
      std::unique_ptr<Geography> merged = other_[i]->Merge(options_);
      std::unique_ptr<Geography> merged_prev = other_[i - 1]->Merge(options_);

      // erase the last two nodes
      other_.erase(other_.begin() + i - 1, other_.begin() + i + 1);

      // ..and replace it with a single node
      other_.push_back(absl::make_unique<Node>());
      other_.back()->index1.Add(*merged);
      other_.back()->index2.Add(*merged_prev);

      // making sure to keep the underlying data alive
      other_.back()->data.push_back(std::move(merged));
      other_.back()->data.push_back(std::move(merged_prev));
    }
  }

  if (other_.size() == 0) {
    return root_.Merge(options_);
  } else {
    std::unique_ptr<Geography> merged = other_[0]->Merge(options_);
    root_.index2.Add(*merged);
    return root_.Merge(options_);
  }
}

}  // namespace s2geography
