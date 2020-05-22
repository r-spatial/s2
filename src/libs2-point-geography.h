
#ifndef LIBS2_POINT_GEOGRAPHY_H
#define LIBS2_POINT_GEOGRAPHY_H

#include "libs2-geography.h"

// This class handles both points and multipoints, as this is how
// points are generally returned/required in S2 (vector of S2Point)
// This is similar to an S2PointVectorLayer
class LibS2PointGeography: public LibS2Geography {
public:
  LibS2PointGeography(): points(0) {}
  LibS2PointGeography(S2Point point): points(1) {
    this->points[0] = point;
  }
  LibS2PointGeography(std::vector<S2Point> points): points(points) {}

  bool IsCollection() {
    return this->points.size() > 1;
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

  S2Point Centroid() {
    S2Point output(0, 0, 0);
    for (size_t i = 0; i < this->points.size(); i++) {
        output += this->points[i];
    }

    return output;
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
      childMeta.hasSize = true;
      childMeta.size = 1;

      handler->nextGeometryStart(meta, partId);

      for (size_t i = 0; i < this->points.size(); i++) {
        point = S2LatLng(this->points[i]);

        handler->nextGeometryStart(childMeta, i);
        handler->nextCoordinate(meta, WKCoord::xy(point.lng().degrees(), point.lat().degrees()), 0);
        handler->nextGeometryEnd(childMeta, i);
      }

      handler->nextGeometryEnd(meta, partId);

    } else if (this->points.size() > 0) {
      // export point
      WKGeometryMeta meta(WKGeometryType::Point, false, false, false);
      meta.hasSize = true;
      meta.size = this->points.size();

      handler->nextGeometryStart(meta, partId);

      point = S2LatLng(this->points[0]);
      handler->nextCoordinate(meta, WKCoord::xy(point.lng().degrees(), point.lat().degrees()), 0);

      handler->nextGeometryEnd(meta, partId);
    } else {
      // export empty point
      // export point
      WKGeometryMeta meta(WKGeometryType::Point, false, false, false);
      meta.hasSize = true;
      meta.size = 0;
      handler->nextGeometryStart(meta, partId);
      handler->nextGeometryEnd(meta, partId);
    }
  }

  class Builder: public LibS2GeographyBuilder {
  public:
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
