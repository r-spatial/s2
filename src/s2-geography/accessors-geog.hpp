
#pragma once

#include "geography.hpp"
#include "aggregator.hpp"

namespace s2geography {

S2Point s2_centroid(const S2Geography& geog);
std::unique_ptr<S2Geography> s2_boundary(const S2Geography& geog);

class S2CentroidAggregator: public S2Aggregator<S2Point> {
public:
    void Add(const S2Geography& geog);
    void Merge(const S2CentroidAggregator& other);
    S2Point Finalize();

private:
    S2Point centroid_;
};

}
