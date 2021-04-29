
#include <Rcpp.h>
#include "wk/rcpp-translate.hpp"
#include "wk/rcpp-coord-reader.hpp"

#include "wk-geography.h"

using namespace Rcpp;

// [[Rcpp::export]]
List cpp_s2_geog_point(NumericVector x, NumericVector y) {
  NumericVector z(x.size());
  z.fill(NA_REAL);
  NumericVector m(x.size());
  m.fill(NA_REAL);

  WKRcppPointCoordProvider provider(x, y, z, m);
  WKRcppPointCoordReader reader(provider);

  WKGeographyWriter writer(provider.nFeatures());
  reader.setHandler(&writer);

  while (reader.hasNextFeature()) {
    checkUserInterrupt();
    reader.iterateFeature();
  }

  return writer.output;
}

// [[Rcpp::export]]
List cpp_s2_make_line(NumericVector x, NumericVector y, IntegerVector featureId) {
  NumericVector z(x.size());
  z.fill(NA_REAL);
  NumericVector m(x.size());
  m.fill(NA_REAL);

  WKRcppLinestringCoordProvider provider(x, y, z, m, featureId);
  WKRcppLinestringCoordReader reader(provider);

  WKGeographyWriter writer(provider.nFeatures());
  reader.setHandler(&writer);

  while (reader.hasNextFeature()) {
    checkUserInterrupt();
    reader.iterateFeature();
  }

  return writer.output;
}

// [[Rcpp::export]]
List cpp_s2_make_polygon(NumericVector x, NumericVector y,
                         IntegerVector featureId, IntegerVector ringId,
                         bool oriented, bool check) {
  NumericVector z(x.size());
  z.fill(NA_REAL);
  NumericVector m(x.size());
  m.fill(NA_REAL);

  WKRcppPolygonCoordProvider provider(x, y, z, m, featureId, ringId);
  WKRcppPolygonCoordReader reader(provider);

  WKGeographyWriter writer(provider.nFeatures());
  writer.setOriented(oriented);
  writer.setCheck(check);

  reader.setHandler(&writer);

  while (reader.hasNextFeature()) {
    checkUserInterrupt();
    reader.iterateFeature();
  }

  if (writer.problemId.size() > 0) {
    Environment s2NS = Environment::namespace_env("s2");
    Function stopProblems = s2NS["stop_problems_create"];
    stopProblems(writer.problemId, writer.problems);
  }

  return writer.output;
}
