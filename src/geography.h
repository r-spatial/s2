
#ifndef GEOGRAPHY_H
#define GEOGRAPHY_H

#include <memory>
#include "s2/s2latlng.h"
#include "s2/s2polyline.h"
#include "s2/s2polygon.h"
#include "s2/s2shape_index.h"
#include "s2/s2shape_index_region.h"
#include "s2/mutable_s2shape_index.h"
#include "s2/s2point_vector_shape.h"
#include "s2/s2cap.h"
#include "wk/geometry-handler.hpp"
#include <Rcpp.h>

#include "s2-geography/s2-geography.hpp"


class Geography {
public:

  enum class Type {
    GEOGRAPHY_EMPTY,
    GEOGRAPHY_POINT,
    GEOGRAPHY_POLYLINE,
    GEOGRAPHY_POLYGON,
    GEOGRAPHY_COLLECTION
  };

  Geography(): hasIndex(false) {}

  virtual std::unique_ptr<s2geography::S2Geography> NewGeography() = 0;

  // every type will build the index differently based on
  // the underlying data, and this can (should?) be done
  // lazily. Returns a vector of shape IDs so the caller
  // can keep track of which shape came from which feature.
  virtual std::vector<int> BuildShapeIndex(MutableS2ShapeIndex* index) = 0;

  // the factory handler is responsible for building these objects
  // but exporting can be done here
  virtual void Export(WKGeometryHandler* handler, uint32_t partId) = 0;

  virtual ~Geography() {}

  // other calculations use ShapeIndex
  virtual S2ShapeIndex* ShapeIndex() {
    if (!this->hasIndex) {
      this->BuildShapeIndex(&this->shape_index_);
      this->hasIndex = true;
    }

    return &this->shape_index_;
  }

protected:
  MutableS2ShapeIndex shape_index_;
  bool hasIndex;
};

#endif
