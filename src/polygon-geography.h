
#ifndef POLYGON_GEOGRAPHY_H
#define POLYGON_GEOGRAPHY_H

#include "wk/reader.hpp"

#include "geography.h"
#include "point-geography.h"
#include "polyline-geography.h"

// This class handles polygons (POLYGON and MULTIPOLYGON)
// This is similar to an S2PolygonLayer
class PolygonGeography: public Geography {
public:
  PolygonGeography() {}
  PolygonGeography(std::unique_ptr<S2Polygon> polygon):
    polygon(std::move(polygon)) {}

  std::unique_ptr<s2geography::S2Geography> NewGeography() {
    return absl::make_unique<s2geography::S2GeographyOwningPolygon>(std::unique_ptr<S2Polygon>(polygon->Clone()));
  }

  std::vector<int> BuildShapeIndex(MutableS2ShapeIndex* index) {
    std::vector<int> shapeIds(1);
    std::unique_ptr<S2Polygon::Shape> shape = absl::make_unique<S2Polygon::Shape>();
    shape->Init(this->polygon.get());
    shapeIds[0] = index->Add(std::move(shape));
    return shapeIds;
  }

  void Export(WKGeometryHandler* handler, uint32_t partId) {
    std::vector<std::vector<int>> flatIndices = this->flatLoopIndices();

    if (flatIndices.size() > 1) {
      // export multipolygon
      WKGeometryMeta meta(WKGeometryType::MultiPolygon, false, false, false);
      meta.hasSize = true;
      meta.size = flatIndices.size();

      WKGeometryMeta childMeta(WKGeometryType::Polygon, false, false, false);
      childMeta.hasSize = true;

      handler->nextGeometryStart(meta, partId);
      for (size_t i = 0; i < flatIndices.size(); i++) {
        childMeta.size = flatIndices[i].size();
        handler->nextGeometryStart(childMeta, i);
        this->exportLoops(handler, childMeta, flatIndices[i]);
        handler->nextGeometryEnd(childMeta, i);
      }

      handler->nextGeometryEnd(meta, partId);

    } else if (flatIndices.size() > 0) {
      // export polygon
      WKGeometryMeta meta(WKGeometryType::Polygon, false, false, false);
      meta.hasSize = true;
      meta.size = flatIndices[0].size();
      handler->nextGeometryStart(meta, partId);
      this->exportLoops(handler, meta, flatIndices[0]);
      handler->nextGeometryEnd(meta, partId);

    } else {
      // export empty polygon
      WKGeometryMeta meta(WKGeometryType::Polygon, false, false, false);
      meta.hasSize = true;
      meta.size = 0;
      handler->nextGeometryStart(meta, partId);
      handler->nextGeometryEnd(meta, partId);
    }
  }

private:
  std::unique_ptr<S2Polygon> polygon;

  // Calculate which loops in the polygon are outer loops (loop->depth() == 0)
  std::vector<int> outerLoopIndices() {
    std::vector<int> indices;
    for (int i = 0; i < this->polygon->num_loops(); i++) {
      if (this->polygon->loop(i)->depth() == 0) {
        indices.push_back(i);
      }
    }

    return indices;
  }

  // Calculate the arrangement of loops in the form of a multipolygon
  // (list(list(shell, !!! holes)))
  std::vector<std::vector<int>> flatLoopIndices() {
    std::vector<int> outerLoops = this->outerLoopIndices();

    std::vector<std::vector<int>> flatIndices(outerLoops.size());
    for (size_t i = 0; i < outerLoops.size(); i++) {
      int k = outerLoops[i];
      flatIndices[i] = std::vector<int>();

      // the first loop here is the shell (depth == 0)
      flatIndices[i].push_back(k);

      // loops in the S2Polygon are arranged such that child loops are
      // directly after the outer loop, so add all loop indices before
      // the next parent loop (or end of polygon). This is similar to
      // S2Polygon::GetLastDescendant() but is slightly easier to understand.
      while (++k < this->polygon->num_loops() && this->polygon->loop(k)->depth() > 0) {
        flatIndices[i].push_back(k);
      }
    }

    return flatIndices;
  }

  void exportLoops(WKGeometryHandler* handler, WKGeometryMeta meta,
                   const std::vector<int>& loopIndices, int loopIdOffset = 0) {
    S2LatLng point;

    for (size_t i = 0; i < loopIndices.size(); i++) {
      int loopId = loopIndices[i];
      S2Loop* loop = this->polygon->loop(loopId);
      if (loop->num_vertices() == 0) {
        continue;
      }

      // this is a slightly ugly way to make it possible to export either the
      // boundaries or the loops using the same code
      WKGeometryMeta childMeta(WKGeometryType::LineString, false, false, false);
      childMeta.hasSize = true;
      childMeta.size = loop->num_vertices() + 1;

      WKGeometryMeta coordMeta;

      if (meta.geometryType == WKGeometryType::Polygon) {
        handler->nextLinearRingStart(meta, loop->num_vertices() + 1, i + loopIdOffset);
        coordMeta = meta;
      } else if (meta.geometryType == WKGeometryType::MultiLineString) {
        handler->nextGeometryStart(childMeta, i + loopIdOffset);
        coordMeta = childMeta;
      } else {
        std::stringstream err;
        err << "Can't export S2Loop with parent geometry type " << meta.geometryType;
        Rcpp::stop(err.str());
      }

      if ((loop->depth() % 2) == 0) {
        // if this is the first ring, use the internal vertex order
        for (int j = 0; j < loop->num_vertices(); j++) {
          point = S2LatLng(loop->vertex(j));
          handler->nextCoordinate(
            coordMeta,
            WKCoord::xy(point.lng().degrees(), point.lat().degrees()),
            j
          );
        }

        // close the loop!
        point = S2LatLng(loop->vertex(0));
        handler->nextCoordinate(
          coordMeta,
          WKCoord::xy(point.lng().degrees(), point.lat().degrees()),
          loop->num_vertices()
        );
      } else {
        // if an interior ring, reverse the vertex order
        for (int j = 0; j < loop->num_vertices(); j++) {
          point = S2LatLng(loop->vertex(loop->num_vertices() - 1 - j));
          handler->nextCoordinate(
            coordMeta,
            WKCoord::xy(point.lng().degrees(), point.lat().degrees()),
            j
          );
        }

        // close the loop!
        point = S2LatLng(loop->vertex(loop->num_vertices() - 1));
        handler->nextCoordinate(
          coordMeta,
          WKCoord::xy(point.lng().degrees(), point.lat().degrees()),
          loop->num_vertices()
        );
      }

      if (meta.geometryType == WKGeometryType::Polygon) {
        handler->nextLinearRingEnd(meta, loop->num_vertices() + 1, i + loopIdOffset);
      } else if (meta.geometryType == WKGeometryType::MultiLineString) {
        handler->nextGeometryEnd(childMeta, i + loopIdOffset);
      }
    }
  }
};

#endif
