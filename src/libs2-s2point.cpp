
#include "s2/s2point.h"
#include "s2/s2latlng.h"
#include <Rcpp.h>
using namespace Rcpp;

// [[Rcpp::export]]
List s2point_from_numeric(NumericVector x, NumericVector y, NumericVector z) {
  List output(x.size());

  for (R_xlen_t i = 0; i < x.size(); i++) {
    output[i] = XPtr<S2Point>(new S2Point(x[i], y[i], z[i]));
  }

  return output;
}

// [[Rcpp::export]]
List s2point_from_s2latlng(List s2latlng) {
  List output(s2latlng.size());

  SEXP item;
  S2Point newItem;
  for (R_xlen_t i = 0; i < s2latlng.size(); i++) {
    item = s2latlng[i];
    if (item == R_NilValue) {
      output[i] = R_NilValue;
    } else {
      XPtr<S2LatLng> ptr(item);
      newItem = ptr->Normalized().ToPoint();
      output[i] = XPtr<S2Point>(new S2Point(newItem));
    }
  }

  return output;
}

// [[Rcpp::export]]
List data_frame_from_s2point(List s2point) {
  NumericVector x(s2point.size());
  NumericVector y(s2point.size());
  NumericVector z(s2point.size());

  SEXP item;
  for (R_xlen_t i = 0; i < s2point.size(); i++) {
    item = s2point[i];
    if (item == R_NilValue) {
      x[i] = NA_REAL;
      y[i] = NA_REAL;
      z[i] = NA_REAL;
    } else {
      XPtr<S2Point> ptr(item);
      x[i] = ptr->x();
      y[i] = ptr->y();
      z[i] = ptr->z();
    }
  }

  return List::create(_["x"] = x, _["y"] = y, _["z"] = z);
}
