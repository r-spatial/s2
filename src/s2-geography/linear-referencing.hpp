
#pragma once

#include "geography.hpp"

namespace s2geography {

double s2_project_normalized(const S2Geography& geog1, const S2Geography& geog2);
S2Point s2_interpolate_normalized(const S2Geography& geog, double distance_norm);

}
