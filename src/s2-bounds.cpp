
#include "s2/s2latlng_rect.h"
#include "s2/s2cap.h"

#include "s2-options.h"
#include "geography-operator.h"

#include <Rcpp.h>
using namespace Rcpp;

// [[Rcpp::export]]
DataFrame cpp_s2_bounds_cap(List geog) {
  SEXP item;
  NumericVector lng(geog.size()), lat(geog.size()), angle(geog.size());

  for (R_xlen_t i = 0; i < geog.size(); i++) {
    Rcpp::checkUserInterrupt();
    item = geog[i];
    if (item == R_NilValue) {
      lat[i] = lng[i] = angle[i] = NA_REAL;
    } else {
      Rcpp::XPtr<RGeography> feature(item);
      S2Cap cap = feature->Geog().Region()->GetCapBound();
      S2LatLng center(cap.center());
      lng[i] = center.lng().degrees();
      lat[i] = center.lat().degrees();
      angle[i] = cap.GetRadius().degrees();
    }
  }

  return DataFrame::create(
    _["lng"] = lng,
    _["lat"] = lat,
    _["angle"] = angle
  );
}

// [[Rcpp::export]]
DataFrame cpp_s2_bounds_rect(List geog) {
  SEXP item;
  NumericVector lng_lo(geog.size()), lat_lo(geog.size()), lng_hi(geog.size()), lat_hi(geog.size());

  for (R_xlen_t i = 0; i < geog.size(); i++) {
    Rcpp::checkUserInterrupt();
    item = geog[i];
    if (item == R_NilValue) {
      lng_lo[i] = lat_lo[i] = lng_hi[i] = lat_hi[i] = NA_REAL;
    } else {
      Rcpp::XPtr<RGeography> feature(item);
      S2LatLngRect rect = feature->Geog().Region()->GetRectBound();
      lng_lo[i] = rect.lng_lo().degrees();
      lat_lo[i] = rect.lat_lo().degrees();
      lng_hi[i] = rect.lng_hi().degrees();
      lat_hi[i] = rect.lat_hi().degrees();
    }
  }

  return DataFrame::create(
    _["lng_lo"] = lng_lo,
    _["lat_lo"] = lat_lo,
    _["lng_hi"] = lng_hi,
    _["lat_hi"] = lat_hi
  );
}
