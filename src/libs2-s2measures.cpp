#include <vector>
#include <memory>
#include <sstream>
#include "s2/s2latlng.h"
#include "s2/s2polyline.h"
#include "s2/s2polygon.h"
#include <Rcpp.h>
using namespace Rcpp;

// [[Rcpp::export]]
NumericVector s2polyline_lengths(List s2polyline, bool degrees = false) {

  SEXP item;
  NumericVector lengths(s2polyline.size());
  for (R_xlen_t i = 0; i < s2polyline.size(); i++) {
    item = s2polyline[i];
    if (item == R_NilValue) {
      lengths(i) = 0.0;
    } else {
      XPtr<S2Polyline> ptr(item);
	  if (degrees)
	    lengths(i) = ptr->GetLength().degrees();
	  else 
	    lengths(i) = ptr->GetLength().radians();
    }
  }
  return lengths;
}

// [[Rcpp::export]]
NumericVector s2polygon_areas(List s2polygon) {

  SEXP item;
  NumericVector areas(s2polygon.size());
  for (R_xlen_t i = 0; i < s2polygon.size(); i++) {
    item = s2polygon[i];
    if (item == R_NilValue) {
      areas(i) = 0.0; // or NA_REAL?
    } else {
      XPtr<S2Polygon> ptr(item);
	  areas(i) = ptr->GetArea();
    }
  }
  return areas;
}
