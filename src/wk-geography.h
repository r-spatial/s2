
#ifndef WK_GEOGRAPHY_H
#define WK_GEOGRAPHY_H

#include "wk/rcpp-io.hpp"
#include "wk/reader.hpp"

#include <Rcpp.h>
#include "geography.h"


class WKGeographyReader: public WKReader {
public:

  WKGeographyReader(WKRcppSEXPProvider& provider):
  WKReader(provider), provider(provider) {}

  void readFeature(size_t featureId) {
    this->handler->nextFeatureStart(featureId);

    if (this->provider.featureIsNull()) {
      this->handler->nextNull(featureId);
    } else {
      Rcpp::XPtr<Geography> geography(this->provider.feature());
      geography->Export(handler, WKReader::PART_ID_NONE);
    }

    this->handler->nextFeatureEnd(featureId);
  }

private:
  WKRcppSEXPProvider& provider;
};

#endif
