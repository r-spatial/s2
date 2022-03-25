
#pragma once

#include "s2/s2boolean_operation.h"

#include "geography.hpp"

namespace s2geography {

bool s2_intersects(const S2GeographyShapeIndex& geog1,
                   const S2GeographyShapeIndex& geog2,
                   const S2BooleanOperation::Options& options);

bool s2_equals(const S2GeographyShapeIndex& geog1,
               const S2GeographyShapeIndex& geog2,
               const S2BooleanOperation::Options& options);

bool s2_contains(const S2GeographyShapeIndex& geog1,
                 const S2GeographyShapeIndex& geog2,
                 const S2BooleanOperation::Options& options);

bool s2_touches(const S2GeographyShapeIndex& geog1,
                const S2GeographyShapeIndex& geog2,
                const S2BooleanOperation::Options& options);

bool s2_intersects_box(const S2GeographyShapeIndex& geog1,
                       S2LatLngRect rect,
                       const S2BooleanOperation::Options& options,
                       double tolerance);

}
