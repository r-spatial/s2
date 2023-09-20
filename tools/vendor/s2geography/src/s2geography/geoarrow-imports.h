
#pragma once

#include <stdint.h>

// These definitions are part of an in-development geometry library
// based on the Arrow C Data interface. Eventually that library
// could be used here (or if it isn't, these concepts are at least
// useful and allowed me to re-use the WKT and WKB readers and
// writers that I refactored to suit that library).

namespace s2geography {

namespace util {

enum Dimensions {
  DIMENSIONS_UNKNOWN = 10000,
  XY = 0,
  XYZ = 1000,
  XYM = 2000,
  XYZM = 3000
};

enum GeometryType {
  GEOMETRY_TYPE_UNKNOWN = 0,
  POINT = 1,
  LINESTRING = 2,
  POLYGON = 3,
  MULTIPOINT = 4,
  MULTILINESTRING = 5,
  MULTIPOLYGON = 6,
  GEOMETRYCOLLECTION = 7
};

}  // namespace util

class Handler {
 public:
  enum Result { CONTINUE = 0, ABORT = 1, ABORT_FEATURE = 2 };

  virtual void new_geometry_type(util::GeometryType /*geometry_type*/) {}
  virtual void new_dimensions(util::Dimensions /*dimensions*/) {}

  virtual Result array_start(const struct ArrowArray* /*array_data*/) {
    return Result::CONTINUE;
  }
  virtual Result feat_start() { return Result::CONTINUE; }
  virtual Result null_feat() { return Result::CONTINUE; }
  virtual Result geom_start(util::GeometryType /*geometry_type*/,
                            int64_t /*size*/) {
    return Result::CONTINUE;
  }
  virtual Result ring_start(int64_t /*size*/) { return Result::CONTINUE; }
  virtual Result coords(const double* /*coord*/, int64_t /*n*/,
                        int32_t /*coord_size*/) {
    return Result::CONTINUE;
  }
  virtual Result ring_end() { return Result::CONTINUE; }
  virtual Result geom_end() { return Result::CONTINUE; }
  virtual Result feat_end() { return Result::CONTINUE; }
  virtual Result array_end() { return Result::CONTINUE; }

  virtual ~Handler() {}
};

}  // namespace s2geography

#define HANDLE_OR_RETURN(expr) \
  result = expr;               \
  if (result != Handler::Result::CONTINUE) return result

#define HANDLE_CONTINUE_OR_BREAK(expr)          \
  result = expr;                                \
  if (result == Handler::Result::ABORT_FEATURE) \
    continue;                                   \
  else if (result == Handler::Result::ABORT)    \
  break
