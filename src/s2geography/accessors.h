
#pragma once

#include "s2geography/geography.h"

namespace s2geography {

bool s2_is_collection(const Geography& geog);
int s2_dimension(const Geography& geog);
int s2_num_points(const Geography& geog);
bool s2_is_empty(const Geography& geog);
double s2_area(const Geography& geog);
double s2_length(const Geography& geog);
double s2_perimeter(const Geography& geog);
double s2_x(const Geography& geog);
double s2_y(const Geography& geog);
bool s2_find_validation_error(const Geography& geog, S2Error* error);

}  // namespace s2geography
