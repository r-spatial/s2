
#pragma once

#include <s2/s2region_coverer.h>

#include "s2geography/geography.h"

namespace s2geography {

S2Point s2_point_on_surface(const Geography& geog, S2RegionCoverer& coverer);
void s2_covering(const Geography& geog, std::vector<S2CellId>* covering,
                 S2RegionCoverer& coverer);
void s2_interior_covering(const Geography& geog,
                          std::vector<S2CellId>* covering,
                          S2RegionCoverer& coverer);
void s2_covering_buffered(const ShapeIndexGeography& geog,
                          double distance_radians,
                          std::vector<S2CellId>* covering,
                          S2RegionCoverer& coverer);

}  // namespace s2geography
