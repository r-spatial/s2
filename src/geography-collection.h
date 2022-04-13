
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

  std::unique_ptr<s2geography::S2Geography> NewGeography() {
    std::vector<std::unique_ptr<s2geography::S2Geography>> features_cpy;
    features_cpy.reserve(features.size());

    for (const auto& feature : features) {
      features_cpy.push_back(feature->NewGeography());
    }

    return absl::make_unique<s2geography::S2GeographyCollection>(std::move(features_cpy));
  }

  Geography::Type GeographyType() {
    return Geography::Type::GEOGRAPHY_COLLECTION;
  }

  bool FindValidationError(S2Error* error) {
    bool result;
    error->Clear();
    for (size_t i = 0; i < this->features.size(); i++) {
      result = this->features[i]->FindValidationError(error);
      if (result) {
        return result;
      }
    }

    return false;
  }

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

  bool IsEmpty() {
    // could also loop and test all(!IsEmpty()), but
    // that is inconsistent with what gets printed
    return this->features.size() == 0;
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

  const std::vector<std::unique_ptr<Geography> >* CollectionFeatures() {
    return &(this->features);
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

private:
  std::vector<std::unique_ptr<Geography>> features;
};

#endif
