
#pragma once

#include "geography.hpp"

namespace s2geography {

double s2_distance(const S2GeographyShapeIndex& geog1, const S2GeographyShapeIndex& geog2);
double s2_max_distance(const S2GeographyShapeIndex& geog1, const S2GeographyShapeIndex& geog2);

}
