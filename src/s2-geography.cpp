
#include "s2/s2latlng.h"
#include "s2/s2polyline.h"
#include "s2/s2polygon.h"

#include "wk/wkb-reader.hpp"
#include "wk/wkt-reader.hpp"
#include "wk/wkb-writer.hpp"
#include "wk/wkt-writer.hpp"
#include "wk/geometry-formatter.hpp"

#include "geography.h"
#include "wk-geography.h"
#include "point-geography.h"
#include "polyline-geography.h"
#include "polygon-geography.h"
#include "geography-collection.h"

#include <Rcpp.h>
using namespace Rcpp;


// [[Rcpp::export]]
List s2_geography_from_wkb(List wkb, bool oriented, bool check) {
  WKRawVectorListProvider provider(wkb);
  WKGeographyWriter writer(wkb.size());
  writer.setOriented(oriented);
  writer.setCheck(check);

  WKBReader reader(provider);
  reader.setHandler(&writer);

  while (reader.hasNextFeature()) {
    checkUserInterrupt();
    reader.iterateFeature();
  }

  return writer.output;
}

// [[Rcpp::export]]
List s2_geography_from_wkt(CharacterVector wkt, bool oriented, bool check) {
  WKCharacterVectorProvider provider(wkt);
  WKGeographyWriter writer(wkt.size());
  writer.setOriented(oriented);
  writer.setCheck(check);

  WKTReader reader(provider);
  reader.setHandler(&writer);

  while (reader.hasNextFeature()) {
    checkUserInterrupt();
    reader.iterateFeature();
  }

  return writer.output;
}

// [[Rcpp::export]]
List s2_geography_full(LogicalVector x) { // create single geography with full polygon
  std::unique_ptr<S2Loop> l = absl::make_unique<S2Loop>(S2Loop::kFull());
  std::unique_ptr<S2Polygon> p = absl::make_unique<S2Polygon>(std::move(l));
  Geography *pg = new PolygonGeography(std::move(p));
  List ret(1);
  ret(0) = Rcpp::XPtr<Geography>(pg);
  return ret;
}

// [[Rcpp::export]]
CharacterVector s2_geography_to_wkt(List s2_geography, int precision, bool trim) {
  WKRcppSEXPProvider provider(s2_geography);
  WKGeographyReader reader(provider);

  WKCharacterVectorExporter exporter(reader.nFeatures());
  exporter.setRoundingPrecision(precision);
  exporter.setTrim(trim);
  WKTWriter writer(exporter);

  reader.setHandler(&writer);
  while (reader.hasNextFeature()) {
    checkUserInterrupt();
    reader.iterateFeature();
  }

  return exporter.output;
}

// [[Rcpp::export]]
List s2_geography_to_wkb(List s2_geography, int endian) {
  WKRcppSEXPProvider provider(s2_geography);
  WKGeographyReader reader(provider);

  WKRawVectorListExporter exporter(reader.nFeatures());
  WKBWriter writer(exporter);
  writer.setEndian(endian);

  reader.setHandler(&writer);
  while (reader.hasNextFeature()) {
    checkUserInterrupt();
    reader.iterateFeature();
  }

  return exporter.output;
}

// [[Rcpp::export]]
CharacterVector s2_geography_format(List s2_geography, int maxCoords, int precision, bool trim) {
  WKRcppSEXPProvider provider(s2_geography);
  WKGeographyReader reader(provider);

  WKCharacterVectorExporter exporter(s2_geography.size());
  exporter.setRoundingPrecision(precision);
  exporter.setTrim(trim);
  WKGeometryFormatter formatter(exporter, maxCoords);

  reader.setHandler(&formatter);
  while (reader.hasNextFeature()) {
    checkUserInterrupt();
    reader.iterateFeature();
  }

  return exporter.output;
}
