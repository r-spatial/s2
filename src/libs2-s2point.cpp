
#include "s2/s2point.h"
#include <Rcpp.h>
using namespace Rcpp;

// [[Rcpp::export]]
List s2point_from_numeric(NumericVector x, NumericVector y, NumericVector z) {
  List output(x.size());

  S2Point item;
  for (R_xlen_t i = 0; i < x.size(); i++) {
    output[i] = XPtr<S2Point>(new S2Point(x[i], y[i], z[i]));
  }

  return output;
}

// [[Rcpp::export]]
List data_frame_from_s2point(List xptr) {
  NumericVector x(xptr.size());
  NumericVector y(xptr.size());
  NumericVector z(xptr.size());

  SEXP item;
  for (R_xlen_t i = 0; i < xptr.size(); i++) {
    item = xptr[i];
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
