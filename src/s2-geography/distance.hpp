
#pragma once

#include "geography.hpp"

namespace s2geography {

double s2_distance(const ShapeIndexGeography& geog1, const ShapeIndexGeography& geog2);
double s2_max_distance(const ShapeIndexGeography& geog1, const ShapeIndexGeography& geog2);
S2Point s2_closest_point(const ShapeIndexGeography& geog1, const ShapeIndexGeography& geog2);
std::pair<S2Point, S2Point> s2_minimum_clearance_line_between(
    const ShapeIndexGeography& geog1, const ShapeIndexGeography& geog2);

}
