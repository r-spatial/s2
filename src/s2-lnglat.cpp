
#include "s2/s2latlng.h"
#include "s2/s2point.h"
#include "wk/rcpp-io.hpp"
#include "wk/wkb-reader.hpp"
#include "wk/wkb-writer.hpp"

#include <Rcpp.h>
using namespace Rcpp;

// [[Rcpp::export]]
List s2_lnglat_from_numeric(NumericVector lng, NumericVector lat) {
  List output(lat.size());

  S2LatLng item;
  for (R_xlen_t i = 0; i < lat.size(); i++) {
    item = S2LatLng::FromDegrees(lat[i], lng[i]);
    output[i] = XPtr<S2LatLng>(new S2LatLng(item));
  }

  return output;
}

// [[Rcpp::export]]
List s2_lnglat_from_s2_point(List s2_point) {
  List output(s2_point.size());

  SEXP item;
  S2LatLng newItem;
  for (R_xlen_t i = 0; i < s2_point.size(); i++) {
    item = s2_point[i];
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
List data_frame_from_s2_lnglat(List xptr) {
  NumericVector lng(xptr.size());
  NumericVector lat(xptr.size());

  SEXP item;
  for (R_xlen_t i = 0; i < xptr.size(); i++) {
    item = xptr[i];
    if (item == R_NilValue) {
      lng[i] = NA_REAL;
      lat[i] = NA_REAL;
    } else {
      XPtr<S2LatLng> ptr(item);
      lng[i] = ptr->lng().degrees();
      lat[i] = ptr->lat().degrees();
    }
  }

  return List::create(_["lng"] = lng, _["lat"] = lat);
}
