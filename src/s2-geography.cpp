
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
  Geography *pg = new PolygonGeography(std::move(p));
  List ret(1);
  ret(0) = Rcpp::XPtr<Geography>(pg);
  return ret;
}
