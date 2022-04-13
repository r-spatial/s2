
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

  std::unique_ptr<s2geography::S2Geography> NewGeography() {
    std::vector<std::unique_ptr<S2Polyline>> polylines_cpy;

    for (const auto& polyline : polylines) {
      polylines_cpy.push_back(std::unique_ptr<S2Polyline>(polyline->Clone()));
    }

    return absl::make_unique<s2geography::S2GeographyOwningPolyline>(std::move(polylines_cpy));
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
      meta.size = 0;
      handler->nextGeometryStart(meta, partId);
      handler->nextGeometryEnd(meta, partId);
    }
  }

private:
  std::vector<std::unique_ptr<S2Polyline>> polylines;
};

#endif
