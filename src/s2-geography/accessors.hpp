
#pragma once

#include "geography.hpp"
#include "aggregator.hpp"

namespace s2geography {

bool s2_is_collection(const S2Geography& geog);
int s2_dimension(const S2Geography& geog);
int s2_num_points(const S2Geography& geog);
bool s2_is_empty(const S2Geography& geog);
double s2_area(const S2Geography& geog);
double s2_length(const S2Geography& geog);
double s2_perimeter(const S2Geography& geog);
double s2_x(const S2Geography& geog);
double s2_y(const S2Geography& geog);

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
