
#ifndef GEOGRAPHY_H
#define GEOGRAPHY_H

#include <memory>
#include "s2/s2latlng.h"
#include "s2/s2polyline.h"
#include "s2/s2polygon.h"
#include "s2/s2shape_index.h"
#include "s2/mutable_s2shape_index.h"
#include "s2/s2point_vector_shape.h"
#include "wk/geometry-handler.hpp"
#include <Rcpp.h>

class Geography {
public:

  Geography(): hasIndex(false) {}

  // accessors need to be methods, since their calculation
  // depends on the geometry type

  // returns true for a multi-
  // or geometrycollection type
  virtual bool IsCollection() = 0;
  // Returns 0 for point, 1 for line, 2 for polygon
  virtual int Dimension() = 0;
  // Returns the number of unique points in the input
  virtual int NumPoints() = 0;
  virtual double Area() = 0;
  virtual double Length() = 0;
  virtual double Perimeter() = 0;
  virtual double X() = 0;
  virtual double Y() = 0;
  virtual S2Point Centroid() = 0;
  virtual std::unique_ptr<Geography> Boundary() = 0;

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


class GeographyBuilder: public WKGeometryHandler {
public:
  virtual std::unique_ptr<Geography> build() = 0;
  virtual ~GeographyBuilder() {}
};

#endif
