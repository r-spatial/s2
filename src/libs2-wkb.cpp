
#include "s2/s2latlng.h"
#include "s2/s2polyline.h"
#include "s2/s2polygon.h"
#include "wk/io-rcpp.h"
#include "wk/wkb-reader.h"
#include "wk/geometry-handler.h"

#include <Rcpp.h>
using namespace Rcpp;


class WKS2LatLngWriter: public WKGeometryHandler {
public:
  List s2latlng;
  R_xlen_t featureId;

  WKS2LatLngWriter(R_xlen_t size): s2latlng(size) {}

  virtual void nextFeatureStart(size_t featureId) {
    this->featureId = featureId;
  }

  virtual void nextNull(size_t featureId) {
    s2latlng[featureId] = R_NilValue;
  }

  virtual void nextGeometryStart(const WKGeometryMeta& meta, uint32_t partId) {
    if (meta.geometryType != WKGeometryType::Point) {
      stop("Can't create s2latlng object from an geometry that is not a point");
    } else if(meta.size == 0) {
      stop("Can't create s2latlng object from an empty point");
    }
  }

  virtual void nextCoordinate(const WKGeometryMeta& meta, const WKCoord& coord, uint32_t coordId) {
    S2LatLng feature = S2LatLng::FromDegrees(coord.y, coord.x);
    s2latlng[this->featureId] = XPtr<S2LatLng>(new S2LatLng(feature));
  }
};

// [[Rcpp::export]]
List s2latlng_from_wkb(List wkb) {
  WKRawVectorListProvider provider(wkb);
  WKS2LatLngWriter writer(wkb.size());
  WKBReader reader(provider, writer);

  while (reader.hasNextFeature()) {
    reader.iterateFeature();
  }

  return writer.s2latlng;
}


class WKS2PolylineWriter: public WKGeometryHandler {
public:
  List s2polyline;
  std::vector<S2LatLng> vertices;

  WKS2PolylineWriter(R_xlen_t size): s2polyline(size) {}

  virtual void nextFeatureEnd(size_t featureId) {
    XPtr<S2Polyline> polylinePtr(new S2Polyline());
    polylinePtr->Init(vertices);
    s2polyline[featureId] = polylinePtr;
  }

  virtual void nextNull(size_t featureId) {
    s2polyline[featureId] = R_NilValue;
  }

  virtual void nextGeometryStart(const WKGeometryMeta& meta, uint32_t partId) {
    if (meta.geometryType != WKGeometryType::LineString) {
      stop("Can't create a s2polyline from a geometry that is not a linestring");
    }

    vertices = std::vector<S2LatLng>(meta.size);
  }

  virtual void nextCoordinate(const WKGeometryMeta& meta, const WKCoord& coord, uint32_t coordId) {
    vertices[coordId] = S2LatLng::FromDegrees(coord.y, coord.x);
  }
};

// [[Rcpp::export]]
List s2polyline_from_wkb(List wkb) {
  WKRawVectorListProvider provider(wkb);
  WKS2PolylineWriter writer(wkb.size());
  WKBReader reader(provider, writer);

  while (reader.hasNextFeature()) {
    reader.iterateFeature();
  }

  return writer.s2polyline;
}


class WKS2PolygonWriter: public WKGeometryHandler {
public:
  List s2polygon;
  std::vector<std::unique_ptr<S2Loop>> loops;
  std::vector<S2Point> vertices;
  bool check;
  bool oriented;

  WKS2PolygonWriter(R_xlen_t size): s2polygon(size), check(true), oriented(false) {}

  virtual void nextFeatureStart(size_t featureId) {
    loops.clear();
  }

  virtual void nextGeometryStart(const WKGeometryMeta& meta, uint32_t partId) {
    if (meta.geometryType != WKGeometryType::Polygon) {
      stop("Can't create a s2poygon from a geometry that is not a polygon");
    }
  }

  virtual void nextLinearRingStart(const WKGeometryMeta& meta, uint32_t size, uint32_t ringId) {
    // skip the last vertex (WKB rings are theoretically closed)
    vertices = std::vector<S2Point>(size - 1);
  }

  virtual void nextCoordinate(const WKGeometryMeta& meta, const WKCoord& coord, uint32_t coordId) {
    if (coordId < vertices.size()) {
      vertices[coordId] = S2LatLng::FromDegrees(coord.y, coord.x).ToPoint();
    }
  }

  virtual void nextLinearRingEnd(const WKGeometryMeta& meta, uint32_t size, uint32_t ringId) {
    loops.push_back(std::unique_ptr<S2Loop>(new S2Loop()));
    ringId = loops.size() - 1;

    loops[ringId]->set_s2debug_override(S2Debug::DISABLE);
    loops[ringId]->Init(vertices);

    // Not sure if && is short-circuiting in C++...
    if (check && !loops[ringId]->IsValid()) {
      S2Error error;
      loops[ringId]->FindValidationError(&error);
      stop(error.text());
    }
  }

  virtual void nextFeatureEnd(size_t featureId) {
    XPtr<S2Polygon> polygon(new S2Polygon());
    if (oriented) {
      polygon->InitOriented(std::move(loops));
    } else {
      polygon->InitNested(std::move(loops));
    }

    s2polygon[featureId] = polygon;
  }

  virtual void nextNull(size_t featureId) {
    s2polygon[featureId] = R_NilValue;
  }
};

// [[Rcpp::export]]
List s2polygon_from_wkb(List wkb) {
  WKRawVectorListProvider provider(wkb);
  WKS2PolygonWriter writer(wkb.size());
  WKBReader reader(provider, writer);

  while (reader.hasNextFeature()) {
    reader.iterateFeature();
  }

  return writer.s2polygon;
}
