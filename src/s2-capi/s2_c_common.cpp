
#include "s2/s2shape.h"
#include "s2/s2region.h"
#include "s2/s2region_union.h"

#include "s2/s2point_region.h"
#include "s2/s2point_vector_shape.h"
#include "s2/s2polyline.h"
#include "s2/s2polygon.h"

#include "s2/mutable_s2shape_index.h"
#include "s2/s2shape_index_region.h"

#include "internal/s2_c_common.hpp"

extern "C" {
// capi func start
s2_status_t* s2_status_create();
void s2_status_destroy(s2_status_t* status);
// capi func end
}

s2_status_t* s2_status_create() {
    return reinterpret_cast<s2_status_t*>(new S2Status());
}

void s2_status_destroy(s2_status_t* status) {
    if (status != nullptr) {
        S2Status* status_cpp = reinterpret_cast<S2Status*>(status);
        delete status_cpp;
    }
}


void S2Geography::GetCellUnionBound(std::vector<S2CellId>* cell_ids) {
    MutableS2ShapeIndex index;
    for (int i = 0; i < num_shapes(); i++) {
        index.Add(Shape(i));
    }

    MakeS2ShapeIndexRegion<MutableS2ShapeIndex>(&index).GetCellUnionBound(cell_ids);
}

std::unique_ptr<S2Shape> S2GeographyOwningPoint::Shape(int id) {
    return absl::make_unique<S2PointVectorShape>(points_);
}

std::unique_ptr<S2Region> S2GeographyOwningPoint::Region() {
    auto region = absl::make_unique<S2RegionUnion>();
    for (const S2Point& point: points_) {
        region->Add(absl::make_unique<S2PointRegion>(point));
    }

    return region;
}

void S2GeographyOwningPoint::GetCellUnionBound(std::vector<S2CellId>* cell_ids) {
    if (points_.size() < 10) {
        for (const S2Point& point: points_) {
            cell_ids->push_back(S2CellId(point));
        }
    } else {
        S2Geography::GetCellUnionBound(cell_ids);
    }
}


int S2GeographyOwningPolyline::num_shapes() { return polylines_.size(); }

std::unique_ptr<S2Shape> S2GeographyOwningPolyline::Shape(int id) {
    return absl::make_unique<S2Polyline::Shape>(polylines_[id].get());
}

std::unique_ptr<S2Region> S2GeographyOwningPolyline::Region() {
    auto region = absl::make_unique<S2RegionUnion>();
    for (const auto& polyline: polylines_) {
        region->Add(std::unique_ptr<S2Region>(polyline->Clone()));
    }
    return region;
}

void S2GeographyOwningPolyline::GetCellUnionBound(std::vector<S2CellId>* cell_ids) {
    for (const auto& polyline: polylines_) {
        polyline->GetCellUnionBound(cell_ids);
    }
}

std::unique_ptr<S2Shape> S2GeographyOwningPolygon::Shape(int id) {
    return absl::make_unique<S2Polygon::Shape>(polygon_.get());
}

std::unique_ptr<S2Region> S2GeographyOwningPolygon::Region() {
    return std::unique_ptr<S2Polygon>(polygon_->Clone());
}

void S2GeographyOwningPolygon::GetCellUnionBound(std::vector<S2CellId>* cell_ids) {
    polygon_->GetCellUnionBound(cell_ids);
}


int S2GeographyCollection::num_shapes() { return total_shapes_; }

std::unique_ptr<S2Shape> S2GeographyCollection::Shape(int id) {
    int sum_shapes_ = 0;
    for (int i = 0; i < total_shapes_; i++) {
        sum_shapes_ += num_shapes_[i];
        if (id < sum_shapes_) {
            return features_[i]->Shape(id - sum_shapes_ + num_shapes_[i]);
        }
    }

    throw S2CAPIError("shape id out of bounds");
}

std::unique_ptr<S2Region> S2GeographyCollection::Region() {
    auto region = absl::make_unique<S2RegionUnion>();
    for (const auto& feature: features_) {
        region->Add(feature->Region());
    }
    return region;
}
