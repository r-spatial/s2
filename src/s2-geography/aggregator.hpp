
#pragma once

#include "geography.hpp"

namespace s2geography {

template <typename ReturnType, typename ...Params>
class S2Aggregator {
public:
    virtual void Add(const S2Geography& geog, Params... parameters) = 0;
    virtual void FinishBatch() {}
    // void Merge(const S2Aggregator<ReturnType, ...Params>& other) = 0;
    virtual ReturnType Finalize() = 0;
};

}
