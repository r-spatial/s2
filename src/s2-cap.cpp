#include "s2/s2boolean_operation.h"
#include "s2/s2closest_edge_query.h"
#include "s2/s2polygon.h"
#include "s2/s2polyline.h"
#include "s2/s2point.h"
#include "s2/s2error.h"
#include "s2/s2boolean_operation.h"
#include "s2/s2builder.h"
#include "s2/s2builderutil_s2polygon_layer.h"
#include "s2/s2builderutil_s2polyline_vector_layer.h"
#include "s2/s2builderutil_s2point_vector_layer.h"
#include "s2/s2builderutil_closed_set_normalizer.h"
#include "s2/s2builderutil_snap_functions.h"
#include "s2/s2shape_index_buffered_region.h"
#include "s2/s2region_coverer.h"
#include "s2/s2latlng_rect.h"
#include "s2/s2cap.h"

#include "s2-options.h"
#include "geography-operator.h"
#include "point-geography.h"
#include "polyline-geography.h"
#include "polygon-geography.h"
#include "geography-collection.h"

#include <Rcpp.h>
using namespace Rcpp;

// [[Rcpp::export]]
DataFrame cpp_s2_cap(List geog) {
  SEXP item;
  NumericVector lat(geog.size()), lng(geog.size()), angle(geog.size());
  for (R_xlen_t i = 0; i < geog.size(); i++) {
    Rcpp::checkUserInterrupt();
    item = geog[i];
    if (item == R_NilValue) {
      lat[i] = lng[i] = angle[i] = NA_REAL;
    } else {
      Rcpp::XPtr<Geography> feature(item);
      // output[i] = this->processFeature(feature, i);
      S2Cap cap = feature->GetCapBound();
      S2LatLng center(cap.center());
      lat[i] = center.lat().degrees();
      lng[i] = center.lng().degrees();
      angle[i] = cap.GetRadius().degrees();
    }
  }
  DataFrame df = DataFrame::create( _["lat"] = lat , _["lng"] = lng, _["angle"] = angle );
  return df;
}

// [[Rcpp::export]]
DataFrame cpp_s2_lat_lng_rect(List geog) {
  SEXP item;
  NumericVector lat_lo(geog.size()), lat_hi(geog.size()), lng_lo(geog.size()), lng_hi(geog.size());
  for (R_xlen_t i = 0; i < geog.size(); i++) {
    Rcpp::checkUserInterrupt();
    item = geog[i];
    if (item == R_NilValue) {
      lat_lo[i] = lat_hi[i] = lng_lo[i] = lng_hi[i] = NA_REAL;
    } else {
      Rcpp::XPtr<Geography> feature(item);
      S2LatLngRect rect = feature->GetRectBound();
      lat_lo[i] = rect.lat_lo().degrees();
      lat_hi[i] = rect.lat_hi().degrees();
      lng_lo[i] = rect.lng_lo().degrees();
      lng_hi[i] = rect.lng_hi().degrees();
    }
  }
  DataFrame df = DataFrame::create( _["lat_lo"] = lat_lo , _["lat_hi"] = lat_hi,
      _["lng_lo"] = lng_lo, _["lng_hi"] = lng_hi );
  return df;
}
