
#include "s2/s2latlng.h"
#include "s2/s2point.h"

#include <Rcpp.h>
using namespace Rcpp;

// [[Rcpp::export]]
List s2_lnglat_from_s2_point(List s2_point) {
  NumericVector x = s2_point[0];
  NumericVector y = s2_point[1];
  NumericVector z = s2_point[2];

  R_len_t n = x.size();
  NumericVector lng(n);
  NumericVector lat(n);

  S2LatLng item;
  for (R_xlen_t i = 0; i < n; i++) {
    item = S2LatLng(S2Point(x[i], y[i], z[i]));
    lng[i] = item.lng().degrees();
    lat[i] = item.lat().degrees();
  }

  return List::create(_["x"] = lng, _["y"] = lat);
}

// [[Rcpp::export]]
List s2_point_from_s2_lnglat(List s2_lnglat) {
  NumericVector lng = s2_lnglat[0];
  NumericVector lat = s2_lnglat[1];

  R_len_t n = lng.size();

  NumericVector x(n);
  NumericVector y(n);
  NumericVector z(n);

  S2Point item;
  for (R_xlen_t i = 0; i < n; i++) {
    item = S2LatLng::FromDegrees(lat[i], lng[i]).Normalized().ToPoint();
    x[i] = item.x();
    y[i] = item.y();
    z[i] = item.z();
  }

  return List::create(_["x"] = x, _["y"] = y, _["z"] = z);
}
