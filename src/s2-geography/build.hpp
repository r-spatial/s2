
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

std::unique_ptr<S2Geography> s2_boolean_operation(const ShapeIndexGeography& geog1,
                                                  const ShapeIndexGeography& geog2,
                                                  S2BooleanOperation::OpType op_type,
                                                  const S2GeographyOptions& options);

std::unique_ptr<S2Geography> s2_unary_union(const ShapeIndexGeography& geog,
                                            const S2GeographyOptions& options);

std::unique_ptr<S2Geography> s2_rebuild(const S2Geography& geog,
                                        const S2GeographyOptions& options);

std::unique_ptr<PointGeography> s2_build_point(const S2Geography& geog);

std::unique_ptr<PolylineGeography> s2_build_polyline(const S2Geography& geog);

std::unique_ptr<PolygonGeography> s2_build_polygon(const S2Geography& geog);

class S2RebuildAggregator: public S2Aggregator<std::unique_ptr<S2Geography>> {
public:
    S2RebuildAggregator(const S2GeographyOptions& options): options_(options) {}
    void Add(const S2Geography& geog);
    std::unique_ptr<S2Geography> Finalize();

private:
    S2GeographyOptions options_;
    ShapeIndexGeography index_;
};

class S2CoverageUnionAggregator: public S2Aggregator<std::unique_ptr<S2Geography>> {
public:
    S2CoverageUnionAggregator(const S2GeographyOptions& options): options_(options) {}

    void Add(const S2Geography& geog);
    std::unique_ptr<S2Geography> Finalize();

private:
    S2GeographyOptions options_;
    ShapeIndexGeography index_;
};

class S2UnionAggregator: public S2Aggregator<std::unique_ptr<S2Geography>> {
public:
    S2UnionAggregator(const S2GeographyOptions& options): options_(options) {}
    void Add(const S2Geography& geog);
    std::unique_ptr<S2Geography> Finalize();

private:
    class Node {
    public:
        ShapeIndexGeography index1;
        ShapeIndexGeography index2;
        std::vector<std::unique_ptr<S2Geography>> data;
        std::unique_ptr<S2Geography> Merge(const S2GeographyOptions& options);
    };

    S2GeographyOptions options_;
    Node root_;
    std::vector<std::unique_ptr<Node>> other_;
};

}
