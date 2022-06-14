
#include "s2/s2latlng.h"
#include "s2/s2polyline.h"
#include "s2/s2polygon.h"

#include "geography.h"

#include <Rcpp.h>
using namespace Rcpp;

// [[Rcpp::export]]
List s2_geography_full(LogicalVector x) { // create single geography with full polygon
  std::unique_ptr<S2Loop> l = absl::make_unique<S2Loop>(S2Loop::kFull());
  std::unique_ptr<S2Polygon> p = absl::make_unique<S2Polygon>(std::move(l));
  return List::create(RGeography::MakeXPtr(RGeography::MakePolygon(std::move(p))));
}

// [[Rcpp::export]]
LogicalVector cpp_s2_geography_is_na(List geog) {
  LogicalVector out(geog.size());
  for (R_xlen_t i = 0; i < geog.size(); i++) {
    out[i] = geog[i] == R_NilValue;
  }
  return out;
}
