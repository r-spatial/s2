
#ifndef WK_GEOGRAPHY_H
#define WK_GEOGRAPHY_H

#include <vector>

#include "wk/rcpp-io.hpp"
#include "wk/reader.hpp"
#include "wk/geometry-handler.hpp"

#include "point-geography.h"
#include "polyline-geography.h"
#include "polygon-geography.h"
#include "geography-collection.h"

#include <Rcpp.h>
#include "geography.h"

#define CODE_HAS_BUILD_ERROR 3938829


class WKGeographyWriter: public WKGeometryHandler {
public:
  Rcpp::List output;
  R_xlen_t featureId;

  Rcpp::IntegerVector problemId;
  Rcpp::CharacterVector problems;

  WKGeographyWriter(R_xlen_t size):
    output(size),
    builder(nullptr),
    oriented(false),
    check(true) {}

  void setOriented(bool oriented) {
    this->oriented = oriented;
  }

  void setCheck(bool check) {
    this->check = check;
  }

  void nextFeatureStart(size_t featureId) {
    this->builder = std::unique_ptr<GeographyBuilder>(nullptr);
    this->featureId = featureId;
  }

  void nextNull(size_t featureId) {
    this->output[featureId] = R_NilValue;
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
        this->builder = absl::make_unique<PolygonGeography::Builder>(
          this->oriented,
          this->check
        );
        break;
      case WKGeometryType::GeometryCollection:
        this->builder = absl::make_unique<GeographyCollection::Builder>(
          this->oriented,
          this->check
        );
        break;
      default:
        std::stringstream err;
        err << "Unknown geometry type in geography builder: " << meta.geometryType;
        this->addProblem(err.str());
        throw WKParseException(CODE_HAS_BUILD_ERROR);
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
    try {
      this->builder->nextLinearRingEnd(meta, size, ringId);
    } catch (WKParseException& e) {
      this->addProblem(e.what());
      throw WKParseException(CODE_HAS_BUILD_ERROR);
    }
  }

  void nextGeometryEnd(const WKGeometryMeta& meta, uint32_t partId) {
    this->builder->nextGeometryEnd(meta, partId);
  }

  void nextFeatureEnd(size_t featureId) {
    if (this->builder) {
      try {
        std::unique_ptr<Geography> feature = builder->build();
        this->output[featureId] = Rcpp::XPtr<Geography>(feature.release());
      } catch (WKParseException& e) {
        this->addProblem(e.what());
        throw WKParseException(CODE_HAS_BUILD_ERROR);
      }
    }
  }

  bool nextError(WKParseException& error, size_t featureId) {
    if (error.code() == CODE_HAS_BUILD_ERROR) {
      this->output[featureId] = R_NilValue;
      return true;
    } else {
      return false;
    }

    this->nextFeatureEnd(featureId);
    return true;
  }

private:
  std::unique_ptr<GeographyBuilder> builder;
  bool oriented;
  bool check;

  void addProblem(std::string what) {
    problemId.push_back(this->featureId);
    problems.push_back(what);
  }
};


class WKGeographyReader: public WKReader {
public:

  WKGeographyReader(WKRcppSEXPProvider& provider):
  WKReader(provider), provider(provider) {}

  void readFeature(size_t featureId) {
    this->handler->nextFeatureStart(featureId);

    if (this->provider.featureIsNull()) {
      this->handler->nextNull(featureId);
    } else {
      Rcpp::XPtr<Geography> geography(this->provider.feature());
      geography->Export(handler, WKReader::PART_ID_NONE);
    }

    this->handler->nextFeatureEnd(featureId);
  }

private:
  WKRcppSEXPProvider& provider;
};

#endif
