
#include "s2/s2latlng.h"
#include "s2/s2polyline.h"
#include "s2/s2polygon.h"
#include "wk/io-rcpp.h"
#include "wk/wkb-reader.h"
#include "wk/wkb-writer.h"
#include "wk/geometry-handler.h"
#include "wk/geometry-debug-handler.h"

#include <Rcpp.h>
using namespace Rcpp;

// -------- importers ----------

class WKS2LatLngWriter: public WKGeometryHandler {
public:
  List s2latlng;
  R_xlen_t featureId;

  WKS2LatLngWriter(R_xlen_t size): s2latlng(size) {}

  void nextFeatureStart(size_t featureId) {
    this->featureId = featureId;
  }

  void nextNull(size_t featureId) {
    s2latlng[featureId] = R_NilValue;
  }

  void nextGeometryStart(const WKGeometryMeta& meta, uint32_t partId) {
    if (meta.geometryType != WKGeometryType::Point) {
      stop("Can't create s2latlng object from an geometry that is not a point");
    } else if(meta.size == 0) {
      stop("Can't create s2latlng object from an empty point");
    }
  }

  void nextCoordinate(const WKGeometryMeta& meta, const WKCoord& coord, uint32_t coordId) {
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

  void nextFeatureEnd(size_t featureId) {
    XPtr<S2Polyline> polylinePtr(new S2Polyline());
    polylinePtr->Init(vertices);
    s2polyline[featureId] = polylinePtr;
  }

  void nextNull(size_t featureId) {
    s2polyline[featureId] = R_NilValue;
  }

  void nextGeometryStart(const WKGeometryMeta& meta, uint32_t partId) {
    if (meta.geometryType != WKGeometryType::LineString) {
      stop("Can't create a s2polyline from a geometry that is not a linestring");
    }

    vertices = std::vector<S2LatLng>(meta.size);
  }

  void nextCoordinate(const WKGeometryMeta& meta, const WKCoord& coord, uint32_t coordId) {
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

  void setOriented(bool oriented) {
    this->oriented = oriented;
  }

  void setCheck(bool check) {
    this->check = check;
  }

  void nextFeatureStart(size_t featureId) {
    loops.clear();
  }

  void nextGeometryStart(const WKGeometryMeta& meta, uint32_t partId) {
    if (meta.geometryType != WKGeometryType::Polygon && meta.geometryType != WKGeometryType::MultiPolygon) {
      stop("Can't create a s2poygon from a geometry that is not a polygon");
    }
  }

  void nextLinearRingStart(const WKGeometryMeta& meta, uint32_t size, uint32_t ringId) {
    // skip the last vertex (WKB rings are theoretically closed)
    vertices = std::vector<S2Point>(size - 1);
  }

  void nextCoordinate(const WKGeometryMeta& meta, const WKCoord& coord, uint32_t coordId) {
    if (coordId < vertices.size()) {
      vertices[coordId] = S2LatLng::FromDegrees(coord.y, coord.x).ToPoint();
    }
  }

  void nextLinearRingEnd(const WKGeometryMeta& meta, uint32_t size, uint32_t ringId) {
    loops.push_back(std::unique_ptr<S2Loop>(new S2Loop()));
    ringId = loops.size() - 1;

    loops[ringId]->set_s2debug_override(S2Debug::DISABLE);
    loops[ringId]->Init(vertices);

    if (!this->oriented)
      loops[ringId]->Normalize();

    // Not sure if && is short-circuiting in C++...
    if (this->check && !loops[ringId]->IsValid()) {
      S2Error error;
      loops[ringId]->FindValidationError(&error);
      stop(error.text());
    }
  }

  void nextFeatureEnd(size_t featureId) {
    XPtr<S2Polygon> polygon(new S2Polygon());
    if (this->oriented) {
      polygon->InitOriented(std::move(loops));
    } else {
      polygon->InitNested(std::move(loops));
    }

    s2polygon[featureId] = polygon;
  }

  void nextNull(size_t featureId) {
    s2polygon[featureId] = R_NilValue;
  }
};

// [[Rcpp::export]]
List s2polygon_from_wkb(List wkb, bool oriented, bool check) {
  WKRawVectorListProvider provider(wkb);
  WKS2PolygonWriter writer(wkb.size());
  writer.setOriented(oriented);
  writer.setCheck(check);
  WKBReader reader(provider, writer);

  while (reader.hasNextFeature()) {
    reader.iterateFeature();
  }

  return writer.s2polygon;
}

// -------- exporters ---------

// this should be added to wk
class WKListProvider: public WKProvider {
public:
  List& input;
  R_xlen_t index;

  WKListProvider(List& input): input(input), index(-1) {}

  SEXP feature() {
    return this->input[this->index];
  }

  bool seekNextFeature() {
    this->index++;
    return this->index < input.size();
  }

  bool featureIsNull() {
    return this->input[this->index] == R_NilValue;
  }

  size_t nFeatures() {
    return input.size();
  }
};

class WKS2LatLngReader: public WKReader {
public:

  WKS2LatLngReader(WKListProvider& provider, WKGeometryHandler& handler):
    WKReader(provider, handler), provider(provider) {}

  void readFeature(size_t featureId) {
    this->handler.nextFeatureStart(featureId);

    if (this->provider.featureIsNull()) {
      this->handler.nextNull(featureId);
    } else {
      this->readItem(this->provider.feature());
    }

    this->handler.nextFeatureEnd(featureId);
  }

  virtual void readItem(SEXP item) {
    WKGeometryMeta meta(WKGeometryType::Point, false, false, false);
    meta.hasSize = true;
    meta.size = 1;

    this->handler.nextGeometryStart(meta, PART_ID_NONE);

    XPtr<S2LatLng> ptr(item);
    const WKCoord coord = WKCoord::xy(ptr->lng().degrees(), ptr->lat().degrees());
    this->handler.nextCoordinate(meta, coord, 0);
    this->handler.nextGeometryEnd(meta, PART_ID_NONE);
  }

private:
  WKListProvider& provider;
};

// [[Rcpp::export]]
List wkb_from_s2latlng(List s2latlng, int endian) {
  WKListProvider provider(s2latlng);
  WKRawVectorListExporter exporter(s2latlng.size());
  WKBWriter writer(exporter);
  writer.setEndian(endian);

  WKS2LatLngReader reader(provider, writer);
  while (reader.hasNextFeature()) {
    reader.iterateFeature();
  }

  return exporter.output;
}

class WKS2PolylineReader: public WKS2LatLngReader {
public:
  WKS2PolylineReader(WKListProvider& provider, WKGeometryHandler& handler):
    WKS2LatLngReader(provider, handler)  {}

  virtual void readItem(SEXP item) {
    XPtr<S2Polyline> ptr(item);

    WKGeometryMeta meta(WKGeometryType::LineString, false, false, false);
    meta.hasSize = true;
    meta.size = ptr->num_vertices();

    this->handler.nextGeometryStart(meta, PART_ID_NONE);
    S2LatLng vertex;
    WKCoord coord;
    for (size_t i = 0; i < ptr->num_vertices(); i++) {
      vertex = S2LatLng(ptr->vertex(i));
      coord = WKCoord::xy(vertex.lng().degrees(), vertex.lat().degrees());
      this->handler.nextCoordinate(meta, coord, i);
    }

    this->handler.nextGeometryEnd(meta, PART_ID_NONE);
  }
};

// [[Rcpp::export]]
List wkb_from_s2polyline(List s2polyline, int endian) {
  WKListProvider provider(s2polyline);
  WKRawVectorListExporter exporter(s2polyline.size());
  WKBWriter writer(exporter);
  writer.setEndian(endian);

  WKS2PolylineReader reader(provider, writer);
  while (reader.hasNextFeature()) {
    reader.iterateFeature();
  }

  return exporter.output;
}

bool is_multi_polygon(S2Polygon *p) {
	int n_outer_loops = 0;
	for (int i = 0; i < p->num_loops(); i++) {
		if (p->GetParent(i) == -1)
			n_outer_loops++;
		if (p->GetParent(i) > 0 || n_outer_loops > 1)
			return true;
	}
	return false;
}

// return List with indexes of the POLYGONS, flattening the nested S2 hierarchy
std::vector<std::vector<int>> multi_polygon_order(S2Polygon *p) {

	// FIXME: handle empty MULTIPOLYGON
	if (p->num_loops() == 0)
		stop("empty MULTIPOLYGON not expected");

	// get nested rings kludge into flat simple feature form:
	IntegerVector outer_index(p->num_loops());
	// we know the first one is outer:
	int n_outer = 1;
	outer_index[0] = 0; // first, outer loop

	// go through all others:
	for (int i = 1; i < p->num_loops(); i++) {
		if (p->GetParent(i) == -1) { // -1 indicates a top-level outer ring, so no hole:
			outer_index[i] = n_outer; // the n_outer-th outer 
			n_outer++;
		} else {
			// possibly hole, or a MULTIPOLYGON outer ring inside a hole:
			if (outer_index[p->GetParent(i)] >= 0) { // parent refers to an outer loop: this is a hole
				outer_index[i] = -1 - outer_index[p->GetParent(i)]; // give negative index
			} else { // parent refers to a hole: this must be a nested outer loop
				outer_index[i] = n_outer;
				n_outer++;
			}
		}
	}

	std::vector<std::vector<int>> indices;
	for (int i = 0; i < outer_index.size(); i++) {
		if (outer_index[i] >= 0) { // add outer loop
			std::vector<int> outer;
			outer.push_back(i);
			indices.push_back(outer);
		} else { // add hole, to -outer_index[i]-1
			int j = -outer_index[i] - 1;
			std::vector<int> vec = indices[j];
			vec.push_back(i);
			indices[j] = vec;
		}
	}
	return indices;
}


class WKS2PolygonReader: public WKS2LatLngReader {
public:
  WKS2PolygonReader(WKListProvider& provider, WKGeometryHandler& handler):
    WKS2LatLngReader(provider, handler)  {}

  virtual void readItem(SEXP item) {
    XPtr<S2Polygon> ptr(item);

    if (is_multi_polygon(ptr)) { // MULTIPOLYGON:
      WKGeometryMeta meta(WKGeometryType::MultiPolygon, false, false, false);
      meta.hasSize = true;

      std::vector<std::vector<int>> indices = multi_polygon_order(ptr);

      // FIXME: output indices.size() here
	  meta.size = indices.size(); // ???
      this->handler.nextGeometryStart(meta, PART_ID_NONE); // ???

	  for (int k = 0; k < indices.size(); k++) { // all outer loops
	  	std::vector<int> loop_indices = indices[k]; // this outer ring + holes

        // meta.size = ptr->num_loops();
		meta.size = loop_indices.size();
        this->handler.nextPolygon(meta, loop_indices.size()); // ???

        // this->handler.nextGeometryStart(meta, PART_ID_NONE);
        WKCoord coord;
  
        for (size_t i = 0; i < loop_indices.size(); i++) {
          const S2Loop* loop = ptr->loop(loop_indices[i]);
          uint32_t loopSize = loop->num_vertices();
          // need to close loop for WKB
          if (loop->num_vertices() > 0) {
            loopSize += 1;
          }

          this->handler.nextLinearRingStart(meta, loopSize, i);
    
          for (size_t j = 0; j < loop->num_vertices(); j++) {
            S2LatLng vertex(loop->vertex(j));
            coord = WKCoord::xy(vertex.lng().degrees(), vertex.lat().degrees());
            this->handler.nextCoordinate(meta, coord, j);
          }

          // need to close loop for WKB
          if (loop->num_vertices() > 0) {
            S2LatLng vertex(loop->vertex(0));
            coord = WKCoord::xy(vertex.lng().degrees(), vertex.lat().degrees());
            this->handler.nextCoordinate(meta, coord, loop->num_vertices());
          }

          this->handler.nextLinearRingEnd(meta, loopSize, i);
	    }
        // this->handler.nextGeometryEnd(meta, PART_ID_NONE);
	  }
      this->handler.nextGeometryEnd(meta, PART_ID_NONE);

	} else { // POLYGON:
      WKGeometryMeta meta(WKGeometryType::Polygon, false, false, false);
      meta.hasSize = true;
      meta.size = ptr->num_loops();
  
      this->handler.nextGeometryStart(meta, PART_ID_NONE);
      WKCoord coord;
  
      for (size_t i = 0; i < ptr->num_loops(); i++) {
        const S2Loop* loop = ptr->loop(i);
        uint32_t loopSize = loop->num_vertices();
        // need to close loop for WKB
        if (loop->num_vertices() > 0) {
          loopSize += 1;
        }

        this->handler.nextLinearRingStart(meta, loopSize, i);
  
        for (size_t j = 0; j < loop->num_vertices(); j++) {
          S2LatLng vertex(loop->vertex(j));
          coord = WKCoord::xy(vertex.lng().degrees(), vertex.lat().degrees());
          this->handler.nextCoordinate(meta, coord, j);
        }

        // need to close loop for WKB
        if (loop->num_vertices() > 0) {
          S2LatLng vertex(loop->vertex(0));
          coord = WKCoord::xy(vertex.lng().degrees(), vertex.lat().degrees());
          this->handler.nextCoordinate(meta, coord, loop->num_vertices());
        }

        this->handler.nextLinearRingEnd(meta, loopSize, i);
      }
      this->handler.nextGeometryEnd(meta, PART_ID_NONE);
    } // else POLYGON
  }
};

// [[Rcpp::export]]
List wkb_from_s2polygon(List s2polygon, int endian) {
  WKListProvider provider(s2polygon);
  WKRawVectorListExporter exporter(s2polygon.size());
  WKBWriter writer(exporter);
  writer.setEndian(endian);

  WKS2PolygonReader reader(provider, writer);
  while (reader.hasNextFeature()) {
    reader.iterateFeature();
  }

  return exporter.output;
}
