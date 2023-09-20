
#pragma once

#include <s2/s2builderutil_s2point_vector_layer.h>
#include <s2/s2builderutil_s2polygon_layer.h>
#include <s2/s2builderutil_s2polyline_vector_layer.h>

#include "s2geography/aggregator.h"
#include "s2geography/geography.h"

namespace s2geography {

class GlobalOptions {
 public:
  enum OutputAction {
    OUTPUT_ACTION_INCLUDE,
    OUTPUT_ACTION_IGNORE,
    OUTPUT_ACTION_ERROR
  };

  GlobalOptions()
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

std::unique_ptr<Geography> s2_boolean_operation(
    const ShapeIndexGeography& geog1, const ShapeIndexGeography& geog2,
    S2BooleanOperation::OpType op_type, const GlobalOptions& options);

std::unique_ptr<Geography> s2_unary_union(const ShapeIndexGeography& geog,
                                          const GlobalOptions& options);

std::unique_ptr<Geography> s2_rebuild(const Geography& geog,
                                      const GlobalOptions& options);

std::unique_ptr<PointGeography> s2_build_point(const Geography& geog);

std::unique_ptr<PolylineGeography> s2_build_polyline(const Geography& geog);

std::unique_ptr<PolygonGeography> s2_build_polygon(const Geography& geog);

class RebuildAggregator : public Aggregator<std::unique_ptr<Geography>> {
 public:
  RebuildAggregator(const GlobalOptions& options) : options_(options) {}
  void Add(const Geography& geog);
  std::unique_ptr<Geography> Finalize();

 private:
  GlobalOptions options_;
  ShapeIndexGeography index_;
};

class S2CoverageUnionAggregator
    : public Aggregator<std::unique_ptr<Geography>> {
 public:
  S2CoverageUnionAggregator(const GlobalOptions& options) : options_(options) {}

  void Add(const Geography& geog);
  std::unique_ptr<Geography> Finalize();

 private:
  GlobalOptions options_;
  ShapeIndexGeography index_;
};

class S2UnionAggregator : public Aggregator<std::unique_ptr<Geography>> {
 public:
  S2UnionAggregator(const GlobalOptions& options) : options_(options) {}
  void Add(const Geography& geog);
  std::unique_ptr<Geography> Finalize();

 private:
  class Node {
   public:
    ShapeIndexGeography index1;
    ShapeIndexGeography index2;
    std::vector<std::unique_ptr<Geography>> data;
    std::unique_ptr<Geography> Merge(const GlobalOptions& options);
  };

  GlobalOptions options_;
  Node root_;
  std::vector<std::unique_ptr<Node>> other_;
};

}  // namespace s2geography
