
#ifndef WK_RCPP_COORD_READER_H
#define WK_RCPP_COORD_READER_H

#include <cmath>
#include "wk/io.hpp"
#include "wk/coord.hpp"
#include "wk/reader.hpp"
#include "wk/geometry-meta.hpp"
#include <Rcpp.h>


class WKRcppPointCoordProvider: public WKProvider {
public:

  WKRcppPointCoordProvider(Rcpp::NumericVector x, Rcpp::NumericVector y,
                           Rcpp::NumericVector z, Rcpp::NumericVector m):
    x(x), y(y), z(z), m(m), index(-1) {}

  void readFeature(WKGeometryHandler* handler) {
    if (this->index >= this->nFeatures() || this->index < 0) {
      throw std::runtime_error("attempt to access index out of range");
    }

    if (this->coordEmpty(this->index)) {
      WKGeometryMeta meta(WKGeometryType::Point, 0);
      handler->nextGeometryStart(meta, WKReader::PART_ID_NONE);
      handler->nextGeometryEnd(meta, WKReader::PART_ID_NONE);
    } else {
      WKCoord coord = this->coord(this->index);
      WKGeometryMeta meta(WKGeometryType::Point, 1);
      meta.hasZ = coord.hasZ;
      meta.hasM = coord.hasM;
      handler->nextGeometryStart(meta, WKReader::PART_ID_NONE);
      handler->nextCoordinate(meta, coord, 0);
      handler->nextGeometryEnd(meta, WKReader::PART_ID_NONE);
    }
  }

  WKCoord coord(R_xlen_t i) {
    double xi = x[i];
    double yi = y[i];
    double zi = z[i];
    double mi = m[i];

    if (std::isnan(zi) && std::isnan(mi)) {
      return WKCoord::xy(xi, yi);
    } else if (std::isnan(mi)) {
      return WKCoord::xyz(xi, yi, zi);
    } else if (std::isnan(zi)) {
      return WKCoord::xym(xi, yi, mi);
    } else {
      return WKCoord::xyzm(xi, yi, zi, mi);
    }
  }

  bool coordEmpty(R_xlen_t i) {
    return std::isnan(x[i]) &&
      std::isnan(y[i]) &&
      std::isnan(z[i]) &&
      std::isnan(m[i]);
  }

  virtual bool seekNextFeature() {
    this->index++;

    if (this->index >= this->nFeatures()) {
      return false;
    } else {
      return true;
    }
  }

  virtual size_t nFeatures() {
    return this->x.size();
  }

  bool featureIsNull() {
    return false;
  }

  void reset() {
    this->index = -1;
  }

protected:
  Rcpp::NumericVector x;
  Rcpp::NumericVector y;
  Rcpp::NumericVector z;
  Rcpp::NumericVector m;
  R_xlen_t index;
};


class WKRcppLinestringCoordProvider: public WKRcppPointCoordProvider {
public:

  WKRcppLinestringCoordProvider(Rcpp::NumericVector x, Rcpp::NumericVector y,
                                Rcpp::NumericVector z, Rcpp::NumericVector m,
                                Rcpp::IntegerVector featureId):
    WKRcppPointCoordProvider(x, y, z, m), featureId(featureId), nSizes(-1) {}

  virtual void readFeature(WKGeometryHandler* handler) {
    if (this->index >= this->nFeatures() || this->index < 0) {
      throw std::runtime_error("attempt to access index out of range");
    }

    uint32_t size = this->sizes[this->index];
    R_xlen_t offset = this->offsets[this->index];

    WKCoord firstCoord = this->coord(offset);
    WKGeometryMeta meta(WKGeometryType::LineString, size);
    meta.hasZ = firstCoord.hasZ;
    meta.hasM = firstCoord.hasM;

    handler->nextGeometryStart(meta, WKReader::PART_ID_NONE);
    for (uint32_t i = 0; i < size; i++) {
      WKCoord coord = this->coord(offset + i);
      handler->nextCoordinate(meta, coord, i);
    }
    handler->nextGeometryEnd(meta, WKReader::PART_ID_NONE);
  }

  virtual size_t nFeatures() {
    if (this->nSizes == -1) {
      if (featureId.size() == 0) {
        this->nSizes = 0;
        return this->nSizes;
      }

      R_xlen_t currentSize = 0;
      this->offsets.push_back(0);

      for (R_xlen_t i = 1; i < featureId.length(); i++) {
        currentSize++;

        if (this->featureId[i - 1] != this->featureId[i]) {
          this->sizes.push_back(currentSize);
          currentSize = 0;
          this->offsets.push_back(i);
        }
      }

      this->sizes.push_back(currentSize + 1);
      this->nSizes = this->offsets.size();
    }

    return this->nSizes;
  }

protected:
  Rcpp::IntegerVector featureId;
  R_xlen_t nSizes;
  std::vector<uint32_t> sizes;
  std::vector<R_xlen_t> offsets;
};

class WKRcppPolygonCoordProvider: public WKRcppPointCoordProvider {
public:

  WKRcppPolygonCoordProvider(Rcpp::NumericVector x, Rcpp::NumericVector y,
                             Rcpp::NumericVector z, Rcpp::NumericVector m,
                             Rcpp::IntegerVector featureId,
                             Rcpp::IntegerVector ringId):
  WKRcppPointCoordProvider(x, y, z, m), featureId(featureId), ringId(ringId), nSizes(-1) {}

  virtual void readFeature(WKGeometryHandler* handler) {
    if (this->index >= this->nFeatures() || this->index < 0) {
      throw std::runtime_error("attempt to access index out of range");
    }

    R_xlen_t featureOffset = this->offsets[this->index];

    WKCoord firstCoord = this->coord(featureOffset);
    WKGeometryMeta meta(WKGeometryType::Polygon, this->ringSizes[this->index].size());
    meta.hasZ = firstCoord.hasZ;
    meta.hasM = firstCoord.hasM;

    handler->nextGeometryStart(meta, WKReader::PART_ID_NONE);
    R_xlen_t offset = featureOffset;
    for (uint32_t i = 0; i < meta.size; i++) {
      uint32_t ringSize = this->ringSizes[this->index][i];
      bool ringIsClosed = this->ringClosed[this->index][i];
      uint32_t ringSizeOut = ringSize + !ringIsClosed;
      firstCoord = this->coord(offset);

      handler->nextLinearRingStart(meta, ringSizeOut, i);
      for (uint32_t j = 0; j < ringSize; j++) {
        WKCoord coord = this->coord(offset + j);
        handler->nextCoordinate(meta, coord, j);
      }

      if (!ringIsClosed) {
        handler->nextCoordinate(meta, firstCoord, ringSize);
      }

      handler->nextLinearRingEnd(meta, ringSize, i);

      offset += ringSize;
    }
    handler->nextGeometryEnd(meta, WKReader::PART_ID_NONE);
  }

  virtual size_t nFeatures() {
    if (this->nSizes == -1) {
      if (featureId.size() == 0) {
        this->nSizes = 0;
        return this->nSizes;
      }

      R_xlen_t currentSize = 0;
      WKCoord firstCoord = this->coord(0);
      std::vector<bool> featureRingClosed;
      std::vector<uint32_t> featureRingSizes;
      this->offsets.push_back(0);

      for (R_xlen_t i = 1; i < featureId.length(); i++) {
        currentSize++;

        bool isRingTransition = currentSize > 1 && this->ringId[i - 1] != this->ringId[i];
        bool isFeatureTransition = this->featureId[i - 1] != this->featureId[i];

        if (isRingTransition || isFeatureTransition) {
          WKCoord lastCoord = this->coord(i - 1);
          featureRingClosed.push_back(lastCoord == firstCoord);
          featureRingSizes.push_back(currentSize);
          currentSize = 0;
          firstCoord = this->coord(i);
        }

        if (isFeatureTransition) {
          this->ringClosed.push_back(std::move(featureRingClosed));
          this->ringSizes.push_back(std::move(featureRingSizes));
          featureRingClosed = std::vector<bool>();
          featureRingSizes = std::vector<uint32_t>();

          this->offsets.push_back(i);
        }
      }

      WKCoord lastCoord = this->coord(featureId.length() - 1);
      featureRingClosed.push_back(lastCoord == firstCoord);
      featureRingSizes.push_back(currentSize + 1);
      this->ringClosed.push_back(std::move(featureRingClosed));
      this->ringSizes.push_back(std::move(featureRingSizes));
      this->nSizes = this->offsets.size();
    }

    return this->nSizes;
  }

protected:
  Rcpp::IntegerVector featureId;
  Rcpp::IntegerVector ringId;
  R_xlen_t nSizes;
  std::vector<std::vector<uint32_t>> ringSizes;
  std::vector<std::vector<bool>> ringClosed;
  std::vector<R_xlen_t> offsets;
};

class WKRcppPointCoordReader: public WKReader {
public:
  WKRcppPointCoordReader(WKRcppPointCoordProvider& provider):
    WKReader(provider), provider(provider) {}

  void readFeature(size_t featureId) {
    this->handler->nextFeatureStart(featureId);
    this->provider.readFeature(this->handler);
    this->handler->nextFeatureEnd(featureId);
  }

protected:
  WKRcppPointCoordProvider& provider;
};

class WKRcppLinestringCoordReader: public WKReader {
public:
  WKRcppLinestringCoordReader(WKRcppLinestringCoordProvider& provider):
    WKReader(provider), provider(provider) {}

  void readFeature(size_t featureId) {
    this->handler->nextFeatureStart(featureId);
    this->provider.readFeature(this->handler);
    this->handler->nextFeatureEnd(featureId);
  }

protected:
  WKRcppLinestringCoordProvider& provider;
};

class WKRcppPolygonCoordReader: public WKReader {
public:
  WKRcppPolygonCoordReader(WKRcppPolygonCoordProvider& provider):
    WKReader(provider), provider(provider) {}

  void readFeature(size_t featureId) {
    this->handler->nextFeatureStart(featureId);
    this->provider.readFeature(this->handler);
    this->handler->nextFeatureEnd(featureId);
  }

protected:
  WKRcppPolygonCoordProvider& provider;
};

#endif
