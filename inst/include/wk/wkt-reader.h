
#ifndef WK_WKT_READER_H
#define WK_WKT_READER_H

#include <memory>
#include "wk/wkt-streamer.h"
#include "wk/geometry.h"
#include "wk/reader.h"
#include "wk/io-string.h"
#include "wk/error-formatter.h"
#include "wk/geometry-handler.h"
#include "wk/parse-exception.h"
#include "wk/coord.h"

class WKTReader: public WKReader, private WKGeometryHandler {
public:
  WKTReader(WKStringProvider& provider): WKReader(provider), baseReader(provider), feature(nullptr) {
    this->baseReader.setHandler(this);
  }

  void readFeature(size_t featureId) {
    baseReader.readFeature(featureId);
  }

protected:

  virtual void nextFeatureStart(size_t featureId) {
    this->stack.clear();
    this->handler->nextFeatureStart(featureId);
  }

  virtual void nextNull(size_t featureId) {
    this->handler->nextNull(featureId);
    this->feature = std::unique_ptr<WKGeometry>(nullptr);
  }

  virtual void nextFeatureEnd(size_t featureId) {
    if (this->feature) {
      this->readGeometry(*feature, PART_ID_NONE);
    }
    this->handler->nextFeatureEnd(featureId);
  }

  void readGeometry(const WKGeometry& geometry, uint32_t partId) {
    this->handler->nextGeometryStart(geometry.meta, partId);

    switch (geometry.meta.geometryType) {

    case WKGeometryType::Point:
      this->readPoint((WKPoint&)geometry);
      break;
    case WKGeometryType::LineString:
      this->readLinestring((WKLineString&)geometry);
      break;
    case WKGeometryType::Polygon:
      this->readPolygon((WKPolygon&)geometry);
      break;

    case WKGeometryType::MultiPoint:
    case WKGeometryType::MultiLineString:
    case WKGeometryType::MultiPolygon:
    case WKGeometryType::GeometryCollection:
      this->readCollection((WKCollection&)geometry);
      break;

    default:
      throw WKParseException(
          ErrorFormatter() <<
            "Unrecognized geometry type: " <<
              geometry.meta.geometryType
      );
    }

    this->handler->nextGeometryEnd(geometry.meta, partId);
  }

  virtual void readPoint(const WKPoint& geometry)  {
    for (uint32_t i=0; i < geometry.coords.size(); i++) {
      this->handler->nextCoordinate(geometry.meta, geometry.coords[i], i);
    }
  }

  virtual void readLinestring(const WKLineString& geometry)  {
    for (uint32_t i=0; i < geometry.coords.size(); i++) {
      this->handler->nextCoordinate(geometry.meta, geometry.coords[i], i);
    }
  }

  virtual void readPolygon(const WKPolygon& geometry)  {
    uint32_t nRings = geometry.rings.size();
    for (uint32_t i=0; i < nRings; i++) {
      uint32_t ringSize = geometry.rings[i].size();
      this->handler->nextLinearRingStart(geometry.meta, ringSize, i);

      for (uint32_t j=0; j < ringSize; j++) {
        this->handler->nextCoordinate(geometry.meta, geometry.rings[i][j], j);
      }

      this->handler->nextLinearRingEnd(geometry.meta, ringSize, i);
    }
  }

  virtual void readCollection(const WKCollection& geometry)  {
    for (uint32_t i=0; i < geometry.meta.size; i++) {
      this->readGeometry(*geometry.geometries[i], i);
    }
  }

  void nextGeometryStart(const WKGeometryMeta& meta, uint32_t partId) {
    switch (meta.geometryType) {

    case WKGeometryType::Point:
      this->stack.push_back(std::unique_ptr<WKGeometry>(new WKPoint(meta)));
      break;

    case WKGeometryType::LineString:
      this->stack.push_back(std::unique_ptr<WKGeometry>(new WKLineString(meta)));
      break;

    case WKGeometryType::Polygon:
      this->stack.push_back(std::unique_ptr<WKGeometry>(new WKPolygon(meta)));
      break;

    case WKGeometryType::MultiPoint:
    case WKGeometryType::MultiLineString:
    case WKGeometryType::MultiPolygon:
    case WKGeometryType::GeometryCollection:
      this->stack.push_back(std::unique_ptr<WKGeometry>(new WKCollection(meta)));
      break;

    default:
      throw WKParseException(
          ErrorFormatter() <<
            "Unrecognized geometry type: " <<
              meta.geometryType
      );
    }
  }

  void nextGeometryEnd(const WKGeometryMeta& meta, uint32_t partId) {
    // there is almost certainly a better way to do this
    std::unique_ptr<WKGeometry> currentPtr(this->stack[this->stack.size() - 1].release());
    this->stack.pop_back();

    // set the size meta
    currentPtr->meta.size = currentPtr->size();
    currentPtr->meta.hasSize = true;

    // if the parent is a collection, add this geometry to the collection
    if (stack.size() >= 1) {
      if (WKCollection* parent = dynamic_cast<WKCollection*>(&this->current())){
        parent->geometries.push_back(std::unique_ptr<WKGeometry>(currentPtr.release()));
      }
    } else if (stack.size() == 0) {
      this->feature = std::unique_ptr<WKGeometry>(currentPtr.release());
    }
  }

  void nextLinearRingStart(const WKGeometryMeta& meta, uint32_t size, uint32_t ringId) {
    ((WKPolygon&)this->current()).rings.push_back(WKLinearRing());
  }

  void nextCoordinate(const WKGeometryMeta& meta, const WKCoord& coord, uint32_t coordId) {
    this->current().addCoordinate(coord);
  }

  bool nextError(WKParseException& error, size_t featureId) {
    return this->handler->nextError(error, featureId);
  }

protected:
  WKTStreamer baseReader;
  std::vector<std::unique_ptr<WKGeometry>> stack;
  std::unique_ptr<WKGeometry> feature;
  WKGeometry& current() {
    return *stack[stack.size() - 1];
  }
};

#endif
