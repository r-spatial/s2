
#pragma once

#include "s2/s2builderutil_s2polygon_layer.h"
#include "s2/s2builderutil_s2polyline_vector_layer.h"
#include "s2/s2builderutil_s2point_vector_layer.h"

#include "geography.hpp"

namespace s2geography {

class S2GeographyOptions {
public:
    enum OutputAction {
        OUTPUT_ACTION_INCLUDE,
        OUTPUT_ACTION_IGNORE,
        OUTPUT_ACTION_ERROR
    };

    S2GeographyOptions()
        : point_layer_action(OUTPUT_ACTION_INCLUDE),
          polyline_layer_action(OUTPUT_ACTION_INCLUDE),
          polygon_layer_action(OUTPUT_ACTION_INCLUDE) {}

    S2BooleanOperation::Options boolean_operation;
    S2Builder::Options builder;

    s2builderutil::S2PointVectorLayer::Options point_layer;
    s2builderutil::S2PolylineVectorLayer::Options polyline_layer;
    s2builderutil::S2PolygonLayer::Options polygon_layer;
    OutputAction point_layer_action;
    OutputAction polyline_layer_action;
    OutputAction polygon_layer_action;
};

std::unique_ptr<S2Geography> s2_boolean_operation(const S2GeographyShapeIndex& geog1,
                                                  const S2GeographyShapeIndex& geog2,
                                                  S2BooleanOperation::OpType op_type,
                                                  const S2GeographyOptions& options);

std::unique_ptr<S2Geography> s2_intersection(const S2GeographyShapeIndex& geog1,
                                             const S2GeographyShapeIndex& geog2,
                                             const S2GeographyOptions& options);

std::unique_ptr<S2Geography> s2_union(const S2GeographyShapeIndex& geog1,
                                      const S2GeographyShapeIndex& geog2,
                                      const S2GeographyOptions& options);

std::unique_ptr<S2Geography> s2_difference(const S2GeographyShapeIndex& geog1,
                                           const S2GeographyShapeIndex& geog2,
                                           const S2GeographyOptions& options);

std::unique_ptr<S2Geography> s2_symmetric_difference(const S2GeographyShapeIndex& geog1,
                                                     const S2GeographyShapeIndex& geog2,
                                                     const S2GeographyOptions& options);

std::unique_ptr<S2Geography> s2_rebuild(const S2GeographyShapeIndex& geog,
                                        const S2GeographyOptions& options);

}
