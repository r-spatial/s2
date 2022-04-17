
#ifndef GEOGRAPHY_H
#define GEOGRAPHY_H

#include <memory>
#include <Rcpp.h>

#include "s2-geography/s2-geography.hpp"


class Geography {
public:
  Geography(): hasIndex(false) {}

  virtual std::unique_ptr<s2geography::S2Geography> NewGeography() = 0;

  // every type will build the index differently based on
  // the underlying data, and this can (should?) be done
  // lazily. Returns a vector of shape IDs so the caller
  // can keep track of which shape came from which feature.
  virtual std::vector<int> BuildShapeIndex(MutableS2ShapeIndex* index) = 0;

  virtual ~Geography() {}

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

class PointGeography: public Geography {
public:
  PointGeography(): points(0) {}
  PointGeography(S2Point point): points(1) {
    this->points[0] = point;
  }
  PointGeography(std::vector<S2Point> points): points(points) {}

  std::unique_ptr<s2geography::S2Geography> NewGeography() {
    return absl::make_unique<s2geography::S2GeographyOwningPoint>(points);
  }

  std::vector<int> BuildShapeIndex(MutableS2ShapeIndex* index) {
    std::vector<int> shapeIds(1);
    std::vector<S2Point> pointsCopy(this->points);

    shapeIds[0] = index->Add(std::unique_ptr<S2PointVectorShape>(
      new S2PointVectorShape(std::move(pointsCopy)))
    );
    return shapeIds;
  }

private:
  std::vector<S2Point> points;
};

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

private:
  std::vector<std::unique_ptr<S2Polyline>> polylines;
};

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

private:
  std::unique_ptr<S2Polygon> polygon;
};

class GeographyCollection: public Geography {
public:
  GeographyCollection(): features(0) {}
  GeographyCollection(std::vector<std::unique_ptr<Geography>> features):
    features(std::move(features)) {}

  std::unique_ptr<s2geography::S2Geography> NewGeography() {
    std::vector<std::unique_ptr<s2geography::S2Geography>> features_cpy;
    features_cpy.reserve(features.size());

    for (const auto& feature : features) {
      features_cpy.push_back(feature->NewGeography());
    }

    return absl::make_unique<s2geography::S2GeographyCollection>(std::move(features_cpy));
  }

  std::vector<int> BuildShapeIndex(MutableS2ShapeIndex* index) {
    std::vector<int> shapeIds;
    for (size_t i = 0; i < this->features.size(); i++) {
      std::vector<int> newShapeIds = this->features[i]->BuildShapeIndex(index);
      for (size_t j = 0; j < newShapeIds.size(); j++) {
        shapeIds.push_back(newShapeIds[j]);
      }
    }
    return shapeIds;
  }

private:
  std::vector<std::unique_ptr<Geography>> features;
};

#endif
