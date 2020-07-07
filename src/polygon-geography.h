
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

  bool IsCollection() {
    return this->outerLoopIndices().size() > 1;
  }

  int Dimension() {
    return 2;
  }

  int NumPoints() {
    return this->polygon->num_vertices();
  }

  bool IsEmpty() {
    return this->polygon->is_empty();
  }

  double Area() {
    return this->polygon->GetArea();
  }

  double Length() {
    return 0;
  }

  double Perimeter() {
    std::unique_ptr<Geography> boundary = this->Boundary();
    return boundary->Length();
  }

  double X() {
    Rcpp::stop("Can't compute X value of a non-point geography");
  }

  double Y() {
    Rcpp::stop("Can't compute Y value of a non-point geography");
  }

  S2Point Centroid() {
    return this->polygon->GetCentroid();
  }

  S2Cap GetCapBound() {
	return this->polygon->GetCapBound();
  }

  S2LatLngRect GetRectBound() {
	return this->polygon->GetRectBound();
  }

  std::unique_ptr<Geography> Boundary() {
    PolylineGeography::Builder builder;
    std::vector<std::vector<int>> flatIndices = this->flatLoopIndices();

    // export multilinestring
    WKGeometryMeta meta(WKGeometryType::MultiLineString, false, false, false);
    meta.hasSize = true;
    meta.size = this->polygon->num_loops();

    builder.nextGeometryStart(meta, WKReader::PART_ID_NONE);
    int loopId = 0;
    for (int i = 0; i < flatIndices.size(); i++) {
      this->exportLoops(&builder, meta, flatIndices[i], loopId);
      loopId += flatIndices[i].size();
    }
    builder.nextGeometryEnd(meta, WKReader::PART_ID_NONE);

    return builder.build();
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
      for (int i = 0; i < flatIndices.size(); i++) {
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

  class Builder: public GeographyBuilder {
  public:
    Builder(bool oriented, bool check):
      oriented(oriented), check(check) {}

    void nextLinearRingStart(const WKGeometryMeta& meta, uint32_t size, uint32_t ringId) {
      // skip the last vertex (WKB rings are theoretically closed)
      if (size > 0) {
        this->vertices = std::vector<S2Point>(size - 1);
      } else {
        this->vertices = std::vector<S2Point>();
      }
    }

    void nextCoordinate(const WKGeometryMeta& meta, const WKCoord& coord, uint32_t coordId) {
      if (coordId < this->vertices.size()) {
        vertices[coordId] = S2LatLng::FromDegrees(coord.y, coord.x).Normalized().ToPoint();
      }
    }

    void nextLinearRingEnd(const WKGeometryMeta& meta, uint32_t size, uint32_t ringId) {
      std::unique_ptr<S2Loop> loop = absl::make_unique<S2Loop>();
      loop->set_s2debug_override(S2Debug::DISABLE);
      loop->Init(vertices);

      if (!oriented) {
        loop->Normalize();
      }

      if (this->check && !loop->IsValid()) {
        std::stringstream err;
        err << "Loop " << (this->loops.size()) << " is not valid: ";
        S2Error error;
        loop->FindValidationError(&error);
        err << error.text();
        Rcpp::stop(err.str());
      }

      this->loops.push_back(std::move(loop));
    }

    std::unique_ptr<Geography> build() {
      std::unique_ptr<S2Polygon> polygon = absl::make_unique<S2Polygon>();
      polygon->set_s2debug_override(S2Debug::DISABLE);
      if (this->loops.size() > 0 && oriented) {
        polygon->InitOriented(std::move(this->loops));
      } else if (this->loops.size() > 0) {
        polygon->InitNested(std::move(this->loops));
      }

      // make sure polygon is valid
      if (this->check && !polygon->IsValid()) {
        S2Error error;
        polygon->FindValidationError(&error);
        Rcpp::stop(error.text());
      }

      return absl::make_unique<PolygonGeography>(std::move(polygon));
    }

  private:
    bool oriented;
    bool check;
    std::vector<S2Point> vertices;
    std::vector<std::unique_ptr<S2Loop>> loops;
  };

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
    for (int i = 0; i < outerLoops.size(); i++) {
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
