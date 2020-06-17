
#ifndef WK_GEOMETRY_H
#define WK_GEOMETRY_H

#include <vector>
#include <cstdint>
#include "wk/geometry-meta.hpp"
#include "wk/coord.hpp"

class WKGeometry {
public:
  WKGeometry(WKGeometryMeta meta): meta(meta) {}
  virtual ~WKGeometry() {}
  WKGeometryMeta meta;
  virtual uint32_t size() = 0;
  virtual void addCoordinate(const WKCoord& coord) = 0;
};

class WKPoint: public WKGeometry {
public:
  WKPoint(WKGeometryMeta meta): WKGeometry(meta) {}
  std::vector<WKCoord> coords;

  uint32_t size() {
    return coords.size();
  }

  void addCoordinate(const WKCoord& coord) {
    coords.push_back(coord);
  }
};

class WKLineString: public WKGeometry {
public:
  WKLineString(WKGeometryMeta meta): WKGeometry(meta) {}
  std::vector<WKCoord> coords;

  uint32_t size() {
    return coords.size();
  }

  void addCoordinate(const WKCoord& coord) {
    coords.push_back(coord);
  }
};

class WKLinearRing: public std::vector<WKCoord> {};

class WKPolygon: public WKGeometry {
public:
  WKPolygon(WKGeometryMeta meta): WKGeometry(meta) {}
  std::vector<WKLinearRing> rings;

  uint32_t size() {
    return rings.size();
  }

  void addCoordinate(const WKCoord& coord) {
    rings[rings.size() - 1].push_back(coord);
  }
};

class WKCollection: public WKGeometry {
public:
  WKCollection(WKGeometryMeta meta): WKGeometry(meta) {}
  std::vector<std::unique_ptr<WKGeometry>> geometries;

  uint32_t size() {
    return geometries.size();
  }

  void addCoordinate(const WKCoord& coord) {
    geometries[geometries.size() - 1]->addCoordinate(coord);
  }
};

#endif
