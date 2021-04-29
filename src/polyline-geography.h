
#ifndef POLYLINE_GEOGRAPHY_H
#define POLYLINE_GEOGRAPHY_H

#include "s2/s2latlng_rect.h"

#include "geography.h"

// This class handles (vectors of) polylines (LINESTRING and MULTILINESTRING)
// This is similar to an S2PolylineVectorLayer
class PolylineGeography: public Geography {
public:
  PolylineGeography(): polylines(0) {}
  PolylineGeography(std::vector<std::unique_ptr<S2Polyline>> polylines):
    polylines(std::move(polylines)) {}

  Geography::Type GeographyType() {
    return Geography::Type::GEOGRAPHY_POLYLINE;
  }

  bool FindValidationError(S2Error* error) {
    bool result;
    error->Clear();
    for (size_t i = 0; i < this->polylines.size(); i++) {
      result = this->polylines[i]->FindValidationError(error);
      if (result) {
        return result;
      }
    }

    return false;
  }

  const std::vector<std::unique_ptr<S2Polyline>>* Polyline() {
    return &(this->polylines);
  }

  bool IsCollection() {
    return this->polylines.size() > 1;
  }

  int Dimension() {
    return 1;
  }

  int NumPoints() {
    int numPoints = 0;
    for (size_t i = 0; i < this->polylines.size(); i++) {
      numPoints += this->polylines[i]->num_vertices();
    }

    return numPoints;
  }

  bool IsEmpty() {
    for (size_t i = 0; i < this->polylines.size(); i++) {
      if (this->polylines[i]->num_vertices() > 0) {
        return false;
      }
    }

    return true;
  }

  double Area() {
    return 0;
  }

  double Length() {
    double length  = 0;
    for (size_t i = 0; i < this->polylines.size(); i++) {
      length += this->polylines[i]->GetLength().radians();
    }

    return length;
  }

  double Perimeter() {
    return 0;
  }

  double X() {
    Rcpp::stop("Can't compute X value of a non-point geography");
  }

  double Y() {
    Rcpp::stop("Can't compute Y value of a non-point geography");
  }

  S2Point Centroid() {
    S2Point output(0, 0, 0);
    for (size_t i = 0; i < this->polylines.size(); i++) {
      output += this->polylines[i]->GetCentroid();
    }

    return output;
  }

  S2LatLngRect GetRectBound() {
	S2LatLngRect rect; 
	if (this->polylines.size())
		rect = this->polylines[0]->GetRectBound();
    for (size_t i = 1; i < this->polylines.size(); i++) {
        rect.Union(this->polylines[i]->GetRectBound()); // depends on order
    }
	return rect;
  }

  std::unique_ptr<Geography> Boundary() {
    std::vector<S2Point> endpoints;
    for (size_t i = 0; i < this->polylines.size(); i++) {
      if (this->polylines[i]->num_vertices() >= 2) {
        endpoints.push_back(this->polylines[i]->vertex(0));
        endpoints.push_back(this->polylines[i]->vertex(1));
      }
    }

    return absl::make_unique<PointGeography>(endpoints);
  }

  std::vector<int> BuildShapeIndex(MutableS2ShapeIndex* index) {
    std::vector<int> shapeIds(this->polylines.size());
    for (size_t i = 0; i < this->polylines.size(); i++) {
      std::unique_ptr<S2Polyline::Shape> shape = absl::make_unique<S2Polyline::Shape>();
      shape->Init(this->polylines[i].get());
      shapeIds[i] = index->Add(std::move(shape));
    }
    return shapeIds;
  }

  void Export(WKGeometryHandler* handler, uint32_t partId) {
    S2LatLng point;

    if (this->polylines.size() > 1) {
      // export multilinestring
      WKGeometryMeta meta(WKGeometryType::MultiLineString, false, false, false);
      meta.hasSize = true;
      meta.size = this->polylines.size();

      handler->nextGeometryStart(meta, partId);

      for (size_t i = 0; i < this->polylines.size(); i++) {
        WKGeometryMeta childMeta(WKGeometryType::LineString, false, false, false);
        childMeta.hasSize = true;
        childMeta.size = this->polylines[i]->num_vertices();

        handler->nextGeometryStart(childMeta, i);

        for (size_t j = 0; j < childMeta.size; j++) {
          point = S2LatLng(this->polylines[i]->vertex(j));
          handler->nextCoordinate(meta, WKCoord::xy(point.lng().degrees(), point.lat().degrees()), j);
        }

        handler->nextGeometryEnd(childMeta, i);
      }

      handler->nextGeometryEnd(meta, partId);

    } else if (this->polylines.size() > 0) {
      // export linestring
      WKGeometryMeta meta(WKGeometryType::LineString, false, false, false);
      meta.hasSize = true;
      meta.size = this->polylines[0]->num_vertices();

      handler->nextGeometryStart(meta, partId);

      for (size_t i = 0; i < meta.size; i++) {
        point = S2LatLng(this->polylines[0]->vertex(i));
        handler->nextCoordinate(meta, WKCoord::xy(point.lng().degrees(), point.lat().degrees()), i);
      }

      handler->nextGeometryEnd(meta, partId);

    } else {
      // export empty linestring
      WKGeometryMeta meta(WKGeometryType::LineString, false, false, false);
      meta.hasSize = true;
      meta.size = this->polylines[0]->num_vertices();
      handler->nextGeometryStart(meta, partId);
      handler->nextGeometryEnd(meta, partId);
    }
  }

  class Builder: public GeographyBuilder {
  public:
    void nextGeometryStart(const WKGeometryMeta& meta, uint32_t partId) {
      if (meta.geometryType == WKGeometryType::LineString) {
        points = std::vector<S2Point>(meta.size);
      }
    }

    void nextCoordinate(const WKGeometryMeta& meta, const WKCoord& coord, uint32_t coordId) {
      points[coordId] = S2LatLng::FromDegrees(coord.y, coord.x).Normalized().ToPoint();
    }

    void nextGeometryEnd(const WKGeometryMeta& meta, uint32_t partId) {
      if (meta.geometryType == WKGeometryType::LineString) {
        polylines.push_back(absl::make_unique<S2Polyline>(std::move(points)));
      }
    }

    std::unique_ptr<Geography> build() {
      return absl::make_unique<PolylineGeography>(std::move(this->polylines));
    }

    private:
      std::vector<S2Point> points;
      std::vector<std::unique_ptr<S2Polyline>> polylines;
  };

private:
  std::vector<std::unique_ptr<S2Polyline>> polylines;
};

#endif
