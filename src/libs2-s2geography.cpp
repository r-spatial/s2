
#include "s2/s2latlng.h"
#include "s2/s2polyline.h"
#include "s2/s2polygon.h"
#include "wk/rcpp-io.h"

#include "wk/wkb-reader.h"
#include "wk/wkt-reader.h"
#include "wk/geometry-formatter.h"
#include "wk/geometry-handler.h"

#include "libs2-s2geography.h"
#include <Rcpp.h>
using namespace Rcpp;


class WKLibS2GeographyWriter: public WKGeometryHandler {
public:
  List s2geography;
  R_xlen_t featureId;

  WKLibS2GeographyWriter(R_xlen_t size): s2geography(size), builder(nullptr) {}

  void nextFeatureStart(size_t featureId) {
    this->builder = std::unique_ptr<LibS2GeographyBuilder>(nullptr);
    this->featureId = featureId;
  }

  void nextNull(size_t featureId) {
    this->s2geography[featureId] = R_NilValue;
  }

  void nextGeometryStart(const WKGeometryMeta& meta, uint32_t partId) {
    if (!this->builder) {
      switch (meta.geometryType) {
      case WKGeometryType::Point:
      case WKGeometryType::MultiPoint:
        this->builder = absl::make_unique<LibS2PointGeography::Builder>();
        break;
      case WKGeometryType::LineString:
      case WKGeometryType::MultiLineString:
        this->builder = absl::make_unique<LibS2PolylineGeography::Builder>();
        break;
      case WKGeometryType::Polygon:
      case WKGeometryType::MultiPolygon:
        this->builder = absl::make_unique<LibS2PolygonGeography::Builder>();
        break;
      default:
        stop("Can't create an S2 geometrycollection (yet)");
      }
    }

    this->builder->nextGeometryStart(meta, partId);
  }

  void nextLinearRingStart(const WKGeometryMeta& meta, uint32_t size, uint32_t ringId) {
    this->builder->nextLinearRingStart(meta, size, ringId);
  }

  void nextCoordinate(const WKGeometryMeta& meta, const WKCoord& coord, uint32_t coordId) {
    this->builder->nextCoordinate(meta, coord, coordId);
  }

  void nextLinearRingEnd(const WKGeometryMeta& meta, uint32_t size, uint32_t ringId) {
    this->builder->nextLinearRingEnd(meta, size, ringId);
  }

  void nextGeometryEnd(const WKGeometryMeta& meta, uint32_t partId) {
    this->builder->nextGeometryEnd(meta, partId);
  }

  void nextFeatureEnd(size_t featureId) {
    if (this->builder) {
      std::unique_ptr<LibS2Geography> feature = builder->build();
      this->s2geography[featureId] = XPtr<LibS2Geography>(feature.release());
    }
  }

private:
  std::unique_ptr<LibS2GeographyBuilder> builder;
};

// [[Rcpp::export]]
List s2geography_from_wkb(List wkb) {
  WKRawVectorListProvider provider(wkb);
  WKLibS2GeographyWriter writer(wkb.size());
  WKBReader reader(provider);
  reader.setHandler(&writer);

  while (reader.hasNextFeature()) {
    reader.iterateFeature();
  }

  return writer.s2geography;
}

// [[Rcpp::export]]
List s2geography_from_wkt(CharacterVector wkt) {
  WKCharacterVectorProvider provider(wkt);
  WKLibS2GeographyWriter writer(wkt.size());
  WKTReader reader(provider);
  reader.setHandler(&writer);

  while (reader.hasNextFeature()) {
    reader.iterateFeature();
  }

  return writer.s2geography;
}

class WKLibS2GeographyReader: public WKReader {
public:

  WKLibS2GeographyReader(WKSEXPProvider& provider):
  WKReader(provider), provider(provider) {}

  void readFeature(size_t featureId) {
    this->handler->nextFeatureStart(featureId);

    if (this->provider.featureIsNull()) {
      this->handler->nextNull(featureId);
    } else {
      XPtr<LibS2Geography> geography(this->provider.feature());
      geography->Export(handler, WKReader::PART_ID_NONE);
    }

    this->handler->nextFeatureEnd(featureId);
  }

private:
  WKSEXPProvider& provider;
};

// [[Rcpp::export]]
CharacterVector s2geography_format(List s2geography, int maxCoords) {
  WKSEXPProvider provider(s2geography);
  WKCharacterVectorExporter exporter(s2geography.size());
  WKGeometryFormatter formatter(exporter, maxCoords);
  WKLibS2GeographyReader reader(provider);
  reader.setHandler(&formatter);

  while (reader.hasNextFeature()) {
    reader.iterateFeature();
  }

  return exporter.output;
}
