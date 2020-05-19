
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
  }

protected:
  MutableS2ShapeIndex shape_index_;
  bool hasIndex;
};

class LibS2GeographyBuilder: public WKGeometryHandler {
public:
  virtual std::unique_ptr<LibS2Geography> build() = 0;
  virtual ~LibS2GeographyBuilder() {}
};

class LibS2PointGeography: public LibS2Geography {
public:
  LibS2PointGeography(): points(0) {}
  LibS2PointGeography(S2Point point): points(1) {
    this->points[0] = point;
  }
  LibS2PointGeography(std::vector<S2Point> points): points(points) {}

  bool IsCollection() {
    return this->NumPoints() > 1;
  }

  int Dimension() {
    return 0;
  }

  int NumPoints() {
    return this->points.size();
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
    if (this->points.size() != 1) {
      return NA_REAL;
    } else {
      S2LatLng latLng(this->points[0]);
      return latLng.lng().degrees();
    }
  }

  double Y() {
    if (this->points.size() != 1) {
      return NA_REAL;
    } else {
      S2LatLng latLng(this->points[0]);
      return latLng.lat().degrees();
    }
  }

  std::unique_ptr<LibS2Geography> Centroid() {
    if (this->NumPoints() == 0) {
      return absl::make_unique<LibS2PointGeography>();
    } else if(this->NumPoints() == 1) {
      return absl::make_unique<LibS2PointGeography>(this->points[0]);
    } else {
      Rcpp::stop("Can't create centroid for more than one point (yet)");
    }
  }

  std::unique_ptr<LibS2Geography> Boundary() {
    return absl::make_unique<LibS2PointGeography>();
  }

  virtual void BuildShapeIndex(MutableS2ShapeIndex* index) {
    std::vector<S2Point> pointsCopy(this->points);
    index->Add(std::unique_ptr<S2PointVectorShape>(new S2PointVectorShape(std::move(points))));
  }

  virtual void Export(WKGeometryHandler* handler, uint32_t partId) {
    S2LatLng point;

    if (this->points.size() > 1) {
      // export multipoint
      WKGeometryMeta meta(WKGeometryType::MultiPoint, false, false, false);
      meta.hasSize = true;
      meta.size = this->points.size();

      WKGeometryMeta childMeta(WKGeometryType::Point, false, false, false);
      meta.hasSize = true;
      meta.size = 1;

      handler->nextGeometryStart(meta, partId);

      for (size_t i = 0; i < this->points.size(); i++) {
        point = S2LatLng(this->points[i]);

        handler->nextGeometryStart(childMeta, i);
        handler->nextCoordinate(meta, WKCoord::xy(point.lng().degrees(), point.lat().degrees()), 0);
        handler->nextGeometryEnd(childMeta, i);
      }

      handler->nextGeometryEnd(meta, partId);

    } else {
      // export point
      WKGeometryMeta meta(WKGeometryType::Point, false, false, false);
      meta.hasSize = true;
      meta.size = this->points.size();

      handler->nextGeometryStart(meta, partId);

      if (this->points.size() > 0) {
        point = S2LatLng(this->points[0]);
        handler->nextCoordinate(meta, WKCoord::xy(point.lng().degrees(), point.lat().degrees()), 0);
      }

      handler->nextGeometryEnd(meta, partId);
    }
  }

  class Builder: public LibS2GeographyBuilder {

    void nextCoordinate(const WKGeometryMeta& meta, const WKCoord& coord, uint32_t coordId) {
      points.push_back(S2LatLng::FromDegrees(coord.y, coord.x).Normalized().ToPoint());
    }

    std::unique_ptr<LibS2Geography> build() {
      return absl::make_unique<LibS2PointGeography>(std::move(this->points));
    }

    private:
      std::vector<S2Point> points;
  };

private:
  std::vector<S2Point> points;
};

#endif
