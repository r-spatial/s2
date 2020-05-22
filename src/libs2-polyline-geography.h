
#ifndef LIBS2_POLYLINE_GEOGRAPHY_H
#define LIBS2_POLYLINE_GEOGRAPHY_H

#include "libs2-geography.h"

// This class handles (vectors of) polylines (LINESTRING and MULTILINESTRING)
// This is similar to an S2PolylineVectorLayer
class LibS2PolylineGeography: public LibS2Geography {
public:
  LibS2PolylineGeography(): polylines(0) {}
  LibS2PolylineGeography(std::vector<std::unique_ptr<S2Polyline>> polylines):
    polylines(std::move(polylines)) {}

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

  std::unique_ptr<LibS2Geography> Centroid() {
    Rcpp::stop("Can't compute centroid for more than one point (yet)");
  }

  std::unique_ptr<LibS2Geography> Boundary() {
    Rcpp::stop("Can't compute boundary for more than one point (yet)");
  }

  virtual void BuildShapeIndex(MutableS2ShapeIndex* index) {
    for (size_t i = 0; i < this->polylines.size(); i++) {
      std::unique_ptr<S2Polyline::Shape> shape = absl::make_unique<S2Polyline::Shape>();
      shape->Init(this->polylines[i].get());
      index->Add(std::move(shape));
    }
  }

  virtual void Export(WKGeometryHandler* handler, uint32_t partId) {
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

  class Builder: public LibS2GeographyBuilder {
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

    std::unique_ptr<LibS2Geography> build() {
      return absl::make_unique<LibS2PolylineGeography>(std::move(this->polylines));
    }

    private:
      std::vector<S2Point> points;
      std::vector<std::unique_ptr<S2Polyline>> polylines;
  };

private:
  std::vector<std::unique_ptr<S2Polyline>> polylines;
};

#endif
