
#include "s2/s2latlng.h"
#include "s2/s2polyline.h"
#include "s2/s2polygon.h"

#include "wk/rcpp-io.hpp"
#include "wk/wkb-reader.hpp"
#include "wk/wkt-reader.hpp"
#include "wk/wkb-writer.hpp"
#include "wk/wkt-writer.hpp"
#include "wk/geometry-formatter.hpp"
#include "wk/geometry-handler.hpp"

#include "snap.h"
#include "geography.h"
#include "point-geography.h"
#include "polyline-geography.h"
#include "polygon-geography.h"
#include "geography-collection.h"
#include <Rcpp.h>
using namespace Rcpp;


class WKGeographyWriter: public WKGeometryHandler {
public:
  List s2_geography;
  R_xlen_t featureId;

  WKGeographyWriter(R_xlen_t size): s2_geography(size), builder(nullptr), oriented(false) {}

  void setOriented(bool oriented) {
    this->oriented = oriented;
  }

  void nextFeatureStart(size_t featureId) {
    this->builder = std::unique_ptr<GeographyBuilder>(nullptr);
    this->featureId = featureId;
  }

  void nextNull(size_t featureId) {
    this->s2_geography[featureId] = R_NilValue;
  }

  void nextGeometryStart(const WKGeometryMeta& meta, uint32_t partId) {
    if (!this->builder) {
      switch (meta.geometryType) {
      case WKGeometryType::Point:
      case WKGeometryType::MultiPoint:
        this->builder = absl::make_unique<PointGeography::Builder>();
        break;
      case WKGeometryType::LineString:
      case WKGeometryType::MultiLineString:
        this->builder = absl::make_unique<PolylineGeography::Builder>();
        break;
      case WKGeometryType::Polygon:
      case WKGeometryType::MultiPolygon:
        this->builder = absl::make_unique<PolygonGeography::Builder>(this->oriented);
        break;
      case WKGeometryType::GeometryCollection:
        this->builder = absl::make_unique<GeographyCollection::Builder>(this->oriented);
        break;
      default:
        std::stringstream err;
        err << "Unknown geometry type in geography builder: " << meta.geometryType;
        Rcpp::stop(err.str());
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
      std::unique_ptr<Geography> feature = builder->build();
      this->s2_geography[featureId] = XPtr<Geography>(feature.release());
    }
  }

private:
  std::unique_ptr<GeographyBuilder> builder;
  bool oriented;
};

// [[Rcpp::export]]
List s2_geography_from_wkb(List wkb, bool oriented) {
  WKRawVectorListProvider provider(wkb);
  WKGeographyWriter writer(wkb.size());
  writer.setOriented(oriented);
  WKBReader reader(provider);
  reader.setHandler(&writer);

  while (reader.hasNextFeature()) {
    reader.iterateFeature();
  }

  return writer.s2_geography;
}

// [[Rcpp::export]]
List s2_geography_from_wkt(CharacterVector wkt, bool oriented) {
  WKCharacterVectorProvider provider(wkt);
  WKGeographyWriter writer(wkt.size());
  writer.setOriented(oriented);
  WKTReader reader(provider);
  reader.setHandler(&writer);

  while (reader.hasNextFeature()) {
    reader.iterateFeature();
  }

  return writer.s2_geography;
}

// [[Rcpp::export]]
List s2_geography_full(LogicalVector x) { // create single geography with full polygon
  std::unique_ptr<S2Loop> l = absl::make_unique<S2Loop>(S2Loop::kFull());
  std::unique_ptr<S2Polygon> p = absl::make_unique<S2Polygon>(std::move(l));
  Geography *pg = new PolygonGeography(std::move(p));
  List ret(1);
  ret(0) = Rcpp::XPtr<Geography>(pg);
  return ret;
}

class WKGeographyReader: public WKReader {
public:

  WKGeographyReader(WKRcppSEXPProvider& provider):
  WKReader(provider), provider(provider) {}

  void readFeature(size_t featureId) {
    this->handler->nextFeatureStart(featureId);

    if (this->provider.featureIsNull()) {
      this->handler->nextNull(featureId);
    } else {
      XPtr<Geography> geography(this->provider.feature());
      geography->Export(handler, WKReader::PART_ID_NONE);
    }

    this->handler->nextFeatureEnd(featureId);
  }

private:
  WKRcppSEXPProvider& provider;
};

// [[Rcpp::export]]
CharacterVector s2_geography_to_wkt(List s2_geography, int precision, bool trim) {
  WKRcppSEXPProvider provider(s2_geography);
  WKGeographyReader reader(provider);

  WKCharacterVectorExporter exporter(reader.nFeatures());
  exporter.setRoundingPrecision(precision);
  exporter.setTrim(trim);
  WKTWriter writer(exporter);

  reader.setHandler(&writer);
  while (reader.hasNextFeature()) {
    reader.iterateFeature();
  }

  return exporter.output;
}

// [[Rcpp::export]]
List s2_geography_to_wkb(List s2_geography, int endian) {
  WKRcppSEXPProvider provider(s2_geography);
  WKGeographyReader reader(provider);

  WKRawVectorListExporter exporter(reader.nFeatures());
  WKBWriter writer(exporter);
  writer.setEndian(endian);

  reader.setHandler(&writer);
  while (reader.hasNextFeature()) {
    reader.iterateFeature();
  }

  return exporter.output;
}

// [[Rcpp::export]]
CharacterVector s2_geography_format(List s2_geography, int maxCoords) {
  WKRcppSEXPProvider provider(s2_geography);
  WKGeographyReader reader(provider);

  WKCharacterVectorExporter exporter(s2_geography.size());
  WKGeometryFormatter formatter(exporter, maxCoords);

  reader.setHandler(&formatter);
  while (reader.hasNextFeature()) {
    reader.iterateFeature();
  }

  return exporter.output;
}
