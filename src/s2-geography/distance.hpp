
#pragma once

#include "geography.hpp"

namespace s2geography {

double s2_distance(const S2GeographyShapeIndex& geog1, const S2GeographyShapeIndex& geog2);
double s2_max_distance(const S2GeographyShapeIndex& geog1, const S2GeographyShapeIndex& geog2);
S2Point s2_closest_point(const S2GeographyShapeIndex& geog1, const S2GeographyShapeIndex& geog2);
std::pair<S2Point, S2Point> s2_minimum_clearance_line_between(
    const S2GeographyShapeIndex& geog1, const S2GeographyShapeIndex& geog2);

}
