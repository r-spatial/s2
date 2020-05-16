
#ifndef LIBS2_GEOGRAPHY_H
#define LIBS2_GEOGRAPHY_H

#include <memory>
#include "s2/s2latlng.h"
#include "s2/s2polyline.h"
#include "s2/s2shape_index.h"
#include "s2/mutable_s2shape_index.h"
#include "s2/s2point_vector_shape.h"
#include "wk/geometry-handler.h"
#include <Rcpp.h>

class LibS2Geography {
public:

  LibS2Geography(): hasIndex(false) {}

  // accessors need to be methods, since their calculation
  // depends on the geometry type
  virtual bool IsCollection() = 0;
  virtual int Dimension() = 0;
  virtual int NumPoints() = 0;
  virtual double Area() = 0;
  virtual double Length() = 0;
  virtual double Perimeter() = 0;
  virtual double X() = 0;
  virtual double Y() = 0;
  virtual std::unique_ptr<LibS2Geography> Centroid() = 0;
  virtual std::unique_ptr<LibS2Geography> Boundary() = 0;

  // every type will build the index differently based on
  // the underlying data, and this can (should?) be done
  // lazily
  virtual void BuildShapeIndex(MutableS2ShapeIndex* index) = 0;

  // the factory handler is responsible for building these objects
  // but exporting can be done here
  virtual void Export(WKGeometryHandler* handler, uint32_t partId) = 0;

  virtual ~LibS2Geography() {}

  // other calculations use ShapeIndex
  virtual S2ShapeIndex* ShapeIndex() {
    if (!this->hasIndex) {
      this->BuildShapeIndex(&this->shape_index_);
      this->hasIndex = true;
    }

    return &this->shape_index_;
  };

protected:
  MutableS2ShapeIndex shape_index_;
  bool hasIndex;
};

class LibS2PointGeography: public LibS2Geography {
public:
  LibS2PointGeography(): isEmpty(true) {}
  LibS2PointGeography(S2LatLng point): isEmpty(false), point(point) {}

  bool IsCollection() {
    return false;
  }

  int Dimension() {
    return  0;
  }

  int NumPoints() {
    return !isEmpty;
  }

  double Area() {
    return 0;
  }

  double Length() {
    return 0;
  }

  double Perimeter() {
    return 0;
  }

  double X() {
    if (this->isEmpty) {
      return NA_REAL;
    } else {
      return point.lng().degrees();
    }
  }

  double Y() {
    if (this->isEmpty) {
      return NA_REAL;
    } else {
      return point.lat().degrees();
    }
  }

  std::unique_ptr<LibS2Geography> Centroid() {
    std::unique_ptr<LibS2PointGeography> ptr;

    if (this->isEmpty) {
      ptr = std::unique_ptr<LibS2PointGeography>(new LibS2PointGeography());
    } else {
      ptr = std::unique_ptr<LibS2PointGeography>(new LibS2PointGeography(this->point));
    }

    return std::move(ptr);
  }

  std::unique_ptr<LibS2Geography> Boundary() {
    std::unique_ptr<LibS2PointGeography> ptr(new LibS2PointGeography());
    return std::move(ptr);
  }

  virtual void BuildShapeIndex(MutableS2ShapeIndex* index) {
    if (!this->isEmpty) {
      std::vector<S2Point> points(1);
      points[0] = S2Point(this->point);
      index->Add(std::unique_ptr<S2PointVectorShape>(new S2PointVectorShape(std::move(points))));
    }
  }

  virtual void Export(WKGeometryHandler* handler, uint32_t partId) {
    WKGeometryMeta meta(WKGeometryType::Point, false, false, false);
    meta.hasSize = true;
    meta.size = !this->isEmpty;

    handler->nextGeometryStart(meta, partId);
    if (!this->isEmpty) {
      handler->nextCoordinate(meta, WKCoord::xy(point.lng().degrees(), point.lat().degrees()), 0);
    }
    handler->nextGeometryEnd(meta, partId);
  }

private:
  bool isEmpty;
  S2LatLng point;
};

#endif
