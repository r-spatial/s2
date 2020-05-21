
#ifndef LIBS2_GEOGRAPHY_H
#define LIBS2_GEOGRAPHY_H

#include <memory>
#include "s2/s2latlng.h"
#include "s2/s2polyline.h"
#include "s2/s2polygon.h"
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

  // returns true for a multi-
  // or geometrycollection type
  virtual bool IsCollection() = 0;
  // Returns 0 for point, 1 for line, 2 for polygon
  virtual int Dimension() = 0;
  // Returns the number of unique points in the input
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

  std::unique_ptr<LibS2Geography> Centroid() {
    return absl::make_unique<LibS2PointGeography>(this->polygon->GetCentroid());
  }

  std::unique_ptr<LibS2Geography> Boundary() {
    std::vector<std::unique_ptr<S2Polyline>> loops(this->polygon->num_loops());

    for (int i = 0; i < this->polygon->num_loops(); i++) {
      S2Loop* loop = this->polygon->loop(i);
      if (loop->num_vertices() == 0) {
        loops[i] = absl::make_unique<S2Polyline>();
        continue;
      }

      std::vector<S2Point> vertices(loop->num_vertices() + 1);
      for (int j = 0; j < loop->num_vertices(); j++) {
        vertices[j] = loop->vertex(i);
      }
      vertices[loop->num_vertices()] = loop->vertex(0);

      std::unique_ptr<S2Polyline> polyline = absl::make_unique<S2Polyline>(std::move(vertices));
      loops[i] = std::move(polyline);
    }

    return absl::make_unique<LibS2PolylineGeography>(std::move(loops));
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
      if (this->loops.size() > 0) {
        polygon->InitOriented(std::move(this->loops));
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
    for (int j = 0; j < outerLoops.size(); j++) {
      flatIndices[j] = std::vector<int>(1);
      flatIndices[j][0] = outerLoops[j];
    }

    for (int i = 0; i < this->polygon->num_loops(); i++) {
      int parent = this->polygon->GetParent(i);
      for (int j = 0; j < outerLoops.size(); j++) {
        if (parent == outerLoops[j]) {
          flatIndices[j].push_back(i);
          break;
        }
      }
    }

    return flatIndices;
  }

  void exportLoops(WKGeometryHandler* handler, const WKGeometryMeta& meta, 
                   const std::vector<int>& loopIndices) {
    S2LatLng point;

    for (int i = 0; i < loopIndices.size(); i++) {
      S2Loop* loop = this->polygon->loop(loopIndices[i]);
      if (loop->num_vertices() == 0) {
        continue;
      }

      handler->nextLinearRingStart(meta, loop->num_vertices() + 1, i);

      for (int j = 0; j < loop->num_vertices(); j++) {
        point = S2LatLng(loop->vertex(j));
        handler->nextCoordinate(
          meta,
          WKCoord::xy(point.lng().degrees(), point.lat().degrees()), 
          j
        );
      }

      // close the loop!
      point = S2LatLng(loop->vertex(0));
      handler->nextCoordinate(
        meta, 
        WKCoord::xy(point.lng().degrees(), point.lat().degrees()), 
        loop->num_vertices()
      );

      handler->nextLinearRingEnd(meta, loop->num_vertices() + 1, i);
    }
  }
};

#endif
