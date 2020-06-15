
#include "s2/s2latlng.h"
#include "s2/s2point.h"
#include "wk/rcpp-io.h"
#include "wk/wkb-reader.h"
#include "wk/wkb-writer.h"
#include "snap.h"

#include <Rcpp.h>
using namespace Rcpp;

// [[Rcpp::export]]
List s2_latlng_from_numeric(NumericVector lat, NumericVector lng) {
  List output(lat.size());

  S2LatLng item;
  for (R_xlen_t i = 0; i < lat.size(); i++) {
    item = S2LatLng::FromDegrees(lat[i], lng[i]);
    output[i] = XPtr<S2LatLng>(new S2LatLng(item));
  }

  return output;
}

// [[Rcpp::export]]
List s2_latlng_from_s2point(List s2point) {
  List output(s2point.size());

  SEXP item;
  S2LatLng newItem;
  for (R_xlen_t i = 0; i < s2point.size(); i++) {
    item = s2point[i];
    if (item == R_NilValue) {
      output[i] = R_NilValue;
    } else {
      XPtr<S2Point> ptr(item);
      output[i] = XPtr<S2LatLng>(new S2LatLng(*ptr));
    }
  }

  return output;
}

// [[Rcpp::export]]
List data_frame_from_s2_latlng(List xptr) {
  NumericVector lat(xptr.size());
  NumericVector lng(xptr.size());

  SEXP item;
  for (R_xlen_t i = 0; i < xptr.size(); i++) {
    item = xptr[i];
    if (item == R_NilValue) {
      lat[i] = NA_REAL;
      lng[i] = NA_REAL;
    } else {
      XPtr<S2LatLng> ptr(item);
      lat[i] = ptr->lat().degrees();
      lng[i] = ptr->lng().degrees();
    }
  }

  return List::create(_["lat"] = lat, _["lng"] = lng);
}

// -------- importers ----------

class WKS2LatLngWriter: public WKGeometryHandler {
public:
  List s2_latlng;
  R_xlen_t featureId;

  WKS2LatLngWriter(R_xlen_t size): s2_latlng(size) {}

  void nextFeatureStart(size_t featureId) {
    this->featureId = featureId;
  }

  void nextNull(size_t featureId) {
    s2_latlng[featureId] = R_NilValue;
  }

  void nextGeometryStart(const WKGeometryMeta& meta, uint32_t partId) {
    if (meta.geometryType != WKGeometryType::Point) {
      stop("Can't create s2_latlng object from an geometry that is not a point");
    } else if(meta.size == 0) {
      stop("Can't create s2_latlng object from an empty point");
    }
  }

  void nextCoordinate(const WKGeometryMeta& meta, const WKCoord& coord, uint32_t coordId) {
    S2LatLng feature = S2LatLng::FromDegrees(coord.y, coord.x);
    s2_latlng[this->featureId] = XPtr<S2LatLng>(new S2LatLng(feature));
  }
};

// [[Rcpp::export]]
List s2_latlng_from_wkb(List wkb) {
  WKRawVectorListProvider provider(wkb);
  WKS2LatLngWriter writer(wkb.size());
  WKBReader reader(provider);
  reader.setHandler(&writer);

  while (reader.hasNextFeature()) {
    reader.iterateFeature();
  }

  return writer.s2_latlng;
}

// -------- exporters ---------

class WKS2LatLngReader: public WKReader {
public:

  WKS2LatLngReader(WKSEXPProvider& provider):
  WKReader(provider), provider(provider) {}

  void readFeature(size_t featureId) {
    this->handler->nextFeatureStart(featureId);

    if (this->provider.featureIsNull()) {
      this->handler->nextNull(featureId);
    } else {
      this->readItem(this->provider.feature());
    }

    this->handler->nextFeatureEnd(featureId);
  }

  virtual void readItem(SEXP item) {
    WKGeometryMeta meta(WKGeometryType::Point, false, false, false);
    meta.hasSize = true;
    meta.size = 1;

    this->handler->nextGeometryStart(meta, PART_ID_NONE);

    XPtr<S2LatLng> ptr(item);
    const WKCoord coord = WKCoord::xy(ptr->lng().degrees(), ptr->lat().degrees());
    this->handler->nextCoordinate(meta, coord, 0);
    this->handler->nextGeometryEnd(meta, PART_ID_NONE);
  }

private:
  WKSEXPProvider& provider;
};

// [[Rcpp::export]]
List wkb_from_s2_latlng(List s2_latlng, int endian) {
  WKSEXPProvider provider(s2_latlng);
  WKRawVectorListExporter exporter(s2_latlng.size());
  WKBWriter writer(exporter);
  writer.setEndian(endian);

  WKS2LatLngReader reader(provider);
  reader.setHandler(&writer);

  while (reader.hasNextFeature()) {
    reader.iterateFeature();
  }

  return exporter.output;
}

