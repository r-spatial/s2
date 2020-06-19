
#ifndef GEOGRAPHY_COLLECTION_H
#define GEOGRAPHY_COLLECTION_H

#include <algorithm>
#include "geography.h"

// This class handles collections of other Geography
// objects.
class GeographyCollection: public Geography {
public:
  GeographyCollection(): features(0) {}
  GeographyCollection(std::vector<std::unique_ptr<Geography>> features):
    features(std::move(features)) {}

  bool IsCollection() {
    return this->features.size() > 0;
  }

  int Dimension() {
    int dimension = -1;
    for (size_t i = 0; i < this->features.size(); i++) {
      dimension = std::max<int>(this->features[i]->Dimension(), dimension);
    }

    return dimension;
  }

  int NumPoints() {
    int numPoints = 0;
    for (size_t i = 0; i < this->features.size(); i++) {
      numPoints += this->features[i]->NumPoints();
    }
    return numPoints;
  }

  double Area() {
    double area = 0;
    for (size_t i = 0; i < this->features.size(); i++) {
      area += this->features[i]->Area();
    }
    return area;
  }

  double Length() {
    double length = 0;
    for (size_t i = 0; i < this->features.size(); i++) {
      length += this->features[i]->Length();
    }
    return length;
  }

  double Perimeter() {
    double perimeter = 0;
    for (size_t i = 0; i < this->features.size(); i++) {
      perimeter += this->features[i]->Perimeter();
    }
    return perimeter;
  }

  double X() {
    Rcpp::stop("Can't compute X value of a non-point geography");
  }

  double Y() {
    Rcpp::stop("Can't compute Y value of a non-point geography");
  }

  S2Point Centroid() {
    S2Point cumCentroid(0, 0, 0);
    for (size_t i = 0; i < this->features.size(); i++) {
      S2Point centroid = this->features[i]->Centroid();
      if (centroid.Norm2() > 0) {
        cumCentroid += centroid.Normalize();
      }
    }

    return cumCentroid;
  }

  std::unique_ptr<Geography> Boundary() {
    std::vector<std::unique_ptr<Geography>> featureBoundaries(this->features.size());
    for (size_t i = 0; i < this->features.size(); i++) {
      featureBoundaries[i] = this->features[i]->Boundary();
    }

    return absl::make_unique<GeographyCollection>(std::move(featureBoundaries));
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

  void Export(WKGeometryHandler* handler, uint32_t partId) {
    WKGeometryMeta meta(WKGeometryType::GeometryCollection, false, false, false);
    meta.hasSize = true;
    meta.size = this->features.size();

    handler->nextGeometryStart(meta, partId);
    for (size_t i = 0; i < this->features.size(); i++) {
      this->features[i]->Export(handler, i);
    }
    handler->nextGeometryEnd(meta, partId);
  }

  class Builder: public GeographyBuilder {
  public:

    Builder(bool oriented, bool check):
      metaPtr(nullptr), builderPtr(nullptr), builderMetaPtr(nullptr),
      oriented(oriented), check(check) {}

    virtual void nextGeometryStart(const WKGeometryMeta& meta, uint32_t partId) {
      // if this is the first call, store the meta reference associated with this geometry
      if (this->metaPtr == nullptr) {
        this->metaPtr = (WKGeometryMeta*) &meta;
        return;
      }

      if (!this->builderPtr) {
        // store a reference to the meta associated with this
        // builder so that we know when the corresponding nextGeometryEnd()
        // is called
        this->builderMetaPtr = (WKGeometryMeta*) &meta;

        switch (meta.geometryType) {
        case WKGeometryType::Point:
        case WKGeometryType::MultiPoint:
          this->builderPtr = absl::make_unique<PointGeography::Builder>();
          break;
        case WKGeometryType::LineString:
        case WKGeometryType::MultiLineString:
          this->builderPtr = absl::make_unique<PolylineGeography::Builder>();
          break;
        case WKGeometryType::Polygon:
        case WKGeometryType::MultiPolygon:
          this->builderPtr = absl::make_unique<PolygonGeography::Builder>(
            this->oriented,
            this->check
          );
          break;
        case WKGeometryType::GeometryCollection:
          this->builderPtr = absl::make_unique<GeographyCollection::Builder>(
            this->oriented,
            this->check
          );
          break;
        default:
          std::stringstream err;
          err << "Unknown geometry type in geography builder: " << meta.geometryType;
          Rcpp::stop(err.str());
        }
      }

      this->builder()->nextGeometryStart(meta, partId);
    }

    virtual void nextLinearRingStart(const WKGeometryMeta& meta, uint32_t size, uint32_t ringId) {
      this->builder()->nextLinearRingStart(meta, size, ringId);
    }

    virtual void nextCoordinate(const WKGeometryMeta& meta, const WKCoord& coord, uint32_t coordId) {
      this->builder()->nextCoordinate(meta, coord, coordId);
    }

    virtual void nextLinearRingEnd(const WKGeometryMeta& meta, uint32_t size, uint32_t ringId) {
      this->builder()->nextLinearRingEnd(meta, size, ringId);
    }

    virtual void nextGeometryEnd(const WKGeometryMeta& meta, uint32_t partId) {
      // the end of this GEOMETRYCOLLECTION
      if (&meta == this->metaPtr) {
        return;
      }

      this->builder()->nextGeometryEnd(meta, partId);

      if (&meta == this->builderMetaPtr) {
        std::unique_ptr<Geography> feature = this->builder()->build();
        features.push_back(std::move(feature));
        this->builderPtr = std::unique_ptr<GeographyBuilder>(nullptr);
        this->builderMetaPtr = nullptr;
      }
    }

    std::unique_ptr<Geography> build() {
      return absl::make_unique<GeographyCollection>(std::move(this->features));
    }

  private:
    std::vector<std::unique_ptr<Geography>> features;
    WKGeometryMeta* metaPtr;
    std::unique_ptr<GeographyBuilder> builderPtr;
    WKGeometryMeta* builderMetaPtr;
    bool oriented;
    bool check;

    GeographyBuilder* builder() {
      if (this->builderPtr) {
        return this->builderPtr.get();
      } else {
        Rcpp::stop("Invalid nesting in geometrycollection (can't find nested builder)");
      }
    }
  };

private:
  std::vector<std::unique_ptr<Geography>> features;
};

#endif
