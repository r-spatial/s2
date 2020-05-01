
#include "s2/s2latlng.h"
#include <Rcpp.h>
using namespace Rcpp;

// [[Rcpp::export]]
List s2latlng_from_numeric(NumericVector lat, NumericVector lng) {
  List output(lat.size());

  S2LatLng item;
  for (R_xlen_t i = 0; i < lat.size(); i++) {
    item = S2LatLng::FromDegrees(lat[i], lng[i]);
    output[i] = XPtr<S2LatLng>(new S2LatLng(item));
  }

  return output;
}

// [[Rcpp::export]]
List data_frame_from_s2latlng(List xptr) {
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
