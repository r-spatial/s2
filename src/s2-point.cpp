
#include "s2/s2point.h"
#include "s2/s2latlng.h"
#include <Rcpp.h>
using namespace Rcpp;

// [[Rcpp::export]]
List s2_point_from_numeric(NumericVector x, NumericVector y, NumericVector z) {
  List output(x.size());

  for (R_xlen_t i = 0; i < x.size(); i++) {
    output[i] = XPtr<S2Point>(new S2Point(x[i], y[i], z[i]));
  }

  return output;
}

// [[Rcpp::export]]
List s2_point_from_s2_lnglat(List s2_lnglat) {
  List output(s2_lnglat.size());

  SEXP item;
  S2Point newItem;
  for (R_xlen_t i = 0; i < s2_lnglat.size(); i++) {
    item = s2_lnglat[i];
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
List data_frame_from_s2_point(List s2_point) {
  NumericVector x(s2_point.size());
  NumericVector y(s2_point.size());
  NumericVector z(s2_point.size());

  SEXP item;
  for (R_xlen_t i = 0; i < s2_point.size(); i++) {
    item = s2_point[i];
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

// [[Rcpp::export]]
List s2_point_op1(List e1, List e2, CharacterVector op) {
  List output(e1.size());

  bool add = (op[0] == "+");
  SEXP item1, item2;
  for (R_xlen_t i = 0; i < e1.size(); i++) {
    item1 = e1[i];
    item2 = e2[i];
    if (item1 == R_NilValue || item2 == R_NilValue)
      stop("NULL pointer found");
    XPtr<S2Point> p1(item1);
    XPtr<S2Point> p2(item2);
    S2Point pp1 = *(p1.get());
    S2Point pp2 = *(p2.get());
    if (add)
      pp1 += pp2;
	else
      pp1 -= pp2;
    output[i] = XPtr<S2Point>(new S2Point(pp1));
  }
  return output;
}

// [[Rcpp::export]]
List s2_point_op2(List e1, NumericVector e2, CharacterVector op) {
  List output(e1.size());

  bool mult = (op[0] == "*");
  SEXP item;
  for (R_xlen_t i = 0; i < e1.size(); i++) {
    item = e1[i];
    if (item == R_NilValue)
      stop("NULL pointer found");
    XPtr<S2Point> p(item);
    S2Point pp = *(p.get());
	if (mult)
      pp *= e2[i];
	else
      pp /= e2[i];
    output[i] = XPtr<S2Point>(new S2Point(pp));
  }
  return output;
}
