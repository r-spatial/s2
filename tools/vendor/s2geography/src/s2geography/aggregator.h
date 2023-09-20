
#pragma once

#include "s2geography/geography.h"

namespace s2geography {

template <typename ReturnType, typename... Params>
class Aggregator {
 public:
  virtual void Add(const Geography& geog, Params... parameters) = 0;
  virtual ReturnType Finalize() = 0;
};

}  // namespace s2geography
