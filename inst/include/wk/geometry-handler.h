
#ifndef WK_GEOMETRY_HANDLER_H
#define WK_GEOMETRY_HANDLER_H

#include "wk/coord.h"
#include "wk/parse-exception.h"
#include "wk/geometry-meta.h"

class WKGeometryHandler {
public:

  virtual void nextFeatureStart(size_t featureId) {

  }

  virtual void nextFeatureEnd(size_t featureId) {

  }

  virtual void nextNull(size_t featureId) {

  }

  virtual void nextGeometryStart(const WKGeometryMeta& meta, uint32_t partId) {

  }

  virtual void nextGeometryEnd(const WKGeometryMeta& meta, uint32_t partId) {

  }

  virtual void nextLinearRingStart(const WKGeometryMeta& meta, uint32_t size, uint32_t ringId) {

  }

  virtual void nextLinearRingEnd(const WKGeometryMeta& meta, uint32_t size, uint32_t ringId) {

  }

  virtual void nextCoordinate(const WKGeometryMeta& meta, const WKCoord& coord, uint32_t coordId) {

  }

  virtual bool nextError(WKParseException& error, size_t featureId) {
    return false;
  }

};

#endif
