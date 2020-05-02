
#ifndef WK_READER_H
#define WK_READER_H

#include "wk/geometry-meta.h"
#include "wk/geometry-handler.h"
#include "wk/io.h"

class WKReader {
public:
  const static uint32_t PART_ID_NONE = UINT32_MAX;
  const static uint32_t RING_ID_NONE = UINT32_MAX;
  const static uint32_t COORD_ID_NONE = UINT32_MAX;

  WKReader(WKProvider& provider, WKGeometryHandler& handler):
    handler(handler), featureId(0), provider(provider) {}

  virtual bool hasNextFeature() {
    return this->provider.seekNextFeature();
  }

  virtual void iterateFeature() {
    try {
      this->readFeature(this->featureId);
    } catch (WKParseException& error) {
      if (!handler.nextError(error, this->featureId)) {
        throw error;
      }
    }

    this->featureId++;
  }

  virtual size_t nFeatures() {
    return  this->provider.nFeatures();
  }

protected:
  WKGeometryHandler& handler;
  size_t featureId;

  virtual void readFeature(size_t featureId) = 0;

private:
  WKProvider& provider;
};


#endif
