
#pragma once

#include "s2/s2builderutil_s2polygon_layer.h"
#include "s2/s2builderutil_s2polyline_vector_layer.h"
#include "s2/s2builderutil_s2point_vector_layer.h"

#include "geography.hpp"
#include "aggregator.hpp"

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

std::unique_ptr<S2Geography> s2_rebuild(const S2Geography& geog,
                                        const S2GeographyOptions& options);

std::unique_ptr<S2GeographyOwningPoint> s2_build_point(const S2Geography& geog);

std::unique_ptr<S2GeographyOwningPolyline> s2_build_polyline(const S2Geography& geog);

std::unique_ptr<S2GeographyOwningPolygon> s2_build_polygon(const S2Geography& geog);

class S2RebuildAggregator: public S2Aggregator<std::unique_ptr<S2Geography>> {
public:
    S2RebuildAggregator(const S2GeographyOptions& options): options_(options) {}

    void Add(const S2Geography& geog);
    void FinishBatch();
    void Merge(const S2RebuildAggregator& other);
    std::unique_ptr<S2Geography> Finalize();

private:
    S2GeographyOptions options_;
    S2GeographyShapeIndex index_;
};

}
