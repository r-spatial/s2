
#pragma once

#include "s2/s2region_coverer.h"

#include "geography.hpp"

namespace s2geography {

S2Point s2_point_on_surface(const S2Geography& geog, S2RegionCoverer& coverer);

}
