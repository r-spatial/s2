
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

private:
  std::vector<std::unique_ptr<Geography>> features;
};

#endif
