
#pragma once

#include <s2/s2convex_hull_query.h>

#include "s2geography/aggregator.h"
#include "s2geography/geography.h"

namespace s2geography {

S2Point s2_centroid(const Geography& geog);
std::unique_ptr<Geography> s2_boundary(const Geography& geog);
std::unique_ptr<Geography> s2_convex_hull(const Geography& geog);

class CentroidAggregator : public Aggregator<S2Point> {
 public:
  void Add(const Geography& geog);
  void Merge(const CentroidAggregator& other);
  S2Point Finalize();

 private:
  S2Point centroid_;
};

class S2ConvexHullAggregator
    : public Aggregator<std::unique_ptr<PolygonGeography>> {
 public:
  void Add(const Geography& geog);
  std::unique_ptr<PolygonGeography> Finalize();

 private:
  S2ConvexHullQuery query_;
  std::vector<std::unique_ptr<Geography>> keep_alive_;
};

}  // namespace s2geography
