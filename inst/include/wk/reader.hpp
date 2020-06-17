
#ifndef WK_READER_H
#define WK_READER_H

#include "wk/geometry-meta.hpp"
#include "wk/geometry-handler.hpp"
#include "wk/io.hpp"

class WKReader {
public:
  const static uint32_t PART_ID_NONE = UINT32_MAX;
  const static uint32_t RING_ID_NONE = UINT32_MAX;
  const static uint32_t COORD_ID_NONE = UINT32_MAX;

  WKReader(WKProvider& provider): handler(nullptr), provider(provider) {
    this->reset();
  }

  virtual void reset() {
    this->provider.reset();
    this->featureId = 0;
  }

  virtual void setHandler(WKGeometryHandler* handler) {
    this->handler = handler;
  }

  virtual bool hasNextFeature() {
    return this->provider.seekNextFeature();
  }

  virtual void iterateFeature() {
    // check to make sure there is a valid handler
    if (handler == nullptr) {
      throw std::runtime_error("Unset handler in WKReader::iterateFeature()");
    }

    try {
      this->readFeature(this->featureId);
    } catch (WKParseException& error) {
      if (!handler->nextError(error, this->featureId)) {
        throw error;
      }
    }

    this->featureId++;
  }

  virtual size_t nFeatures() {
    return  this->provider.nFeatures();
  }

protected:
  WKGeometryHandler* handler;
  size_t featureId;

  virtual void readFeature(size_t featureId) = 0;

private:
  WKProvider& provider;
};


#endif
