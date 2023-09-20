
#pragma once

#include "s2geography/geography.h"

namespace s2geography {

double s2_project_normalized(const Geography& geog1, const Geography& geog2);
S2Point s2_interpolate_normalized(const Geography& geog, double distance_norm);

}  // namespace s2geography
