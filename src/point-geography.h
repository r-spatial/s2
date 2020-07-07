
#ifndef POINT_GEOGRAPHY_H
#define POINT_GEOGRAPHY_H

#include "s2/s2latlng_rect.h"

#include "geography.h"

// This class handles both points and multipoints, as this is how
// points are generally returned/required in S2 (vector of S2Point)
// This is similar to an S2PointVectorLayer
class PointGeography: public Geography {
public:
  PointGeography(): points(0) {}
  PointGeography(S2Point point): points(1) {
    this->points[0] = point;
  }
  PointGeography(std::vector<S2Point> points): points(points) {}

  bool IsCollection() {
    return this->points.size() > 1;
  }

  int Dimension() {
    return 0;
  }

  int NumPoints() {
    return this->points.size();
  }

  bool IsEmpty() {
    return this->points.size() == 0;
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

  S2LatLngRect GetRectBound() {
	S2LatLngRect rect; 
    for (size_t i = 0; i < this->points.size(); i++) {
        rect.AddPoint(this->points[i]); // depends on order
    }
	return rect;
  }

  std::unique_ptr<Geography> Boundary() {
    return absl::make_unique<PointGeography>();
  }

  std::vector<int> BuildShapeIndex(MutableS2ShapeIndex* index) {
    std::vector<int> shapeIds(1);
    std::vector<S2Point> pointsCopy(this->points);

    shapeIds[0] = index->Add(std::unique_ptr<S2PointVectorShape>(
      new S2PointVectorShape(std::move(pointsCopy)))
    );
    return shapeIds;
  }

  void Export(WKGeometryHandler* handler, uint32_t partId) {
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

  class Builder: public GeographyBuilder {
  public:
    void nextCoordinate(const WKGeometryMeta& meta, const WKCoord& coord, uint32_t coordId) {
      points.push_back(S2LatLng::FromDegrees(coord.y, coord.x).Normalized().ToPoint());
    }

    std::unique_ptr<Geography> build() {
      return absl::make_unique<PointGeography>(std::move(this->points));
    }

    private:
      std::vector<S2Point> points;
  };

private:
  std::vector<S2Point> points;
};

#endif
