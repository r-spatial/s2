
#ifndef LIBS2_POLYGON_GEOGRAPHY_H
#define LIBS2_POLYGON_GEOGRAPHY_H

#include "libs2-geography.h"
#include "libs2-point-geography.h"
#include "libs2-polyline-geography.h"

// This class handles polygons (POLYGON and MULTIPOLYGON)
// This is similar to an S2PolygonLayer
class LibS2PolygonGeography: public LibS2Geography {
public:
  LibS2PolygonGeography() {}
  LibS2PolygonGeography(std::unique_ptr<S2Polygon> polygon):
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

  double Area() {
    return this->polygon->GetArea();
  }

  double Length() {
    return 0;
  }

  double Perimeter() {
    std::unique_ptr<LibS2Geography> boundary = this->Boundary();
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

  std::unique_ptr<LibS2Geography> Boundary() {
    LibS2PolylineGeography::Builder builder;
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

  void BuildShapeIndex(MutableS2ShapeIndex* index) {
    std::unique_ptr<S2Polygon::Shape> shape = absl::make_unique<S2Polygon::Shape>();
    shape->Init(this->polygon.get());
    index->Add(std::move(shape));
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

  class Builder: public LibS2GeographyBuilder {
  public:
    Builder(bool oriented): oriented(oriented) {}

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

      if (!loop->IsValid()) {
        std::stringstream err;
        err << "Loop " << (this->loops.size()) << " is not valid: ";
        S2Error error;
        loop->FindValidationError(&error);
        err << error.text();
        Rcpp::stop(err.str());
      }

      this->loops.push_back(std::move(loop));
    }

    std::unique_ptr<LibS2Geography> build() {
      std::unique_ptr<S2Polygon> polygon = absl::make_unique<S2Polygon>();
      if (this->loops.size() > 0 && oriented) {
        polygon->InitOriented(std::move(this->loops));
      } else if (this->loops.size() > 0) {
        polygon->InitNested(std::move(this->loops));
      }

      // make sure polygon is valid
      if (!polygon->IsValid()) {
        S2Error error;
        polygon->FindValidationError(&error);
        Rcpp::stop(error.text());
      }

      return absl::make_unique<LibS2PolygonGeography>(std::move(polygon));
    }

  private:
    bool oriented;
    std::vector<S2Point> vertices;
    std::vector<std::unique_ptr<S2Loop>> loops;
  };

private:
  std::unique_ptr<S2Polygon> polygon;

  // Calculate which loops in the polygon are outer loops
  std::vector<int> outerLoopIndices() {
    std::vector<int> indices;
    for (int i = 0; i < this->polygon->num_loops(); i++) {
      if (this->polygon->GetParent(i) == -1) {
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
      int thisLoop = outerLoops[i];
      int lastDescendant = this->polygon->GetLastDescendant(thisLoop);
      flatIndices[i] = std::vector<int>(lastDescendant - i + 1);

      for (size_t j = 0; j < flatIndices[i].size(); j++) {
        flatIndices[i][j] = thisLoop + j;
      }
    }

    return flatIndices;
  }

  void exportLoops(WKGeometryHandler* handler, WKGeometryMeta meta,
                   const std::vector<int>& loopIndices, int loopIdOffset = 0) {
    S2LatLng point;

    for (int i = 0; i < loopIndices.size(); i++) {
      S2Loop* loop = this->polygon->loop(loopIndices[i]);
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
