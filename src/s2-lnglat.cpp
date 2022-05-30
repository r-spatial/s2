
#include "s2/s2latlng.h"
#include "s2/s2point.h"

#include <Rcpp.h>
using namespace Rcpp;

#include "wk-v1.h"

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

int s2_point_trans(R_xlen_t feature_id, const double* xyzm_in, double* xyzm_out, void* trans_data) {
    S2Point pt = S2LatLng::FromDegrees(xyzm_in[1], xyzm_in[0]).Normalized().ToPoint();
    xyzm_out[0] = pt.x();
    xyzm_out[1] = pt.y();
    xyzm_out[2] = pt.z();
    return WK_CONTINUE;
}

void trans_s2_point_lnglat_finalize(void* trans_data) {

}

extern "C" SEXP c_s2_trans_s2_point_new() {
    wk_trans_t* trans = wk_trans_create();
    trans->use_z = 1;

    trans->trans = &s2_point_trans;
    trans->finalizer = &trans_s2_point_lnglat_finalize;
    return wk_trans_create_xptr(trans, R_NilValue, R_NilValue);
}

int s2_lnglat_trans(R_xlen_t feature_id, const double* xyzm_in, double* xyzm_out, void* trans_data) {
    S2LatLng pt(S2Point(xyzm_in[0], xyzm_in[1], xyzm_in[2]));
    xyzm_out[0] = pt.lng().degrees();
    xyzm_out[1] = pt.lat().degrees();
    return WK_CONTINUE;
}

extern "C" SEXP c_s2_trans_s2_lnglat_new() {
    wk_trans_t* trans = wk_trans_create();
    trans->use_z = 0;

    trans->trans = &s2_lnglat_trans;
    trans->finalizer = &trans_s2_point_lnglat_finalize;
    return wk_trans_create_xptr(trans, R_NilValue, R_NilValue);
}
