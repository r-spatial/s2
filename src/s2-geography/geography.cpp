
#include "s2/s2shape.h"
#include "s2/s2region.h"
#include "s2/s2region_union.h"

#include "s2/s2point_region.h"
#include "s2/s2point_vector_shape.h"
#include "s2/s2polyline.h"
#include "s2/s2polygon.h"

#include "s2/mutable_s2shape_index.h"
#include "s2/s2shape_index_region.h"

#include "geography.hpp"
using namespace s2geography;


void S2Geography::GetCellUnionBound(std::vector<S2CellId>* cell_ids) const {
    MutableS2ShapeIndex index;
    for (int i = 0; i < num_shapes(); i++) {
        index.Add(Shape(i));
    }

    MakeS2ShapeIndexRegion<MutableS2ShapeIndex>(&index).GetCellUnionBound(cell_ids);
}

std::unique_ptr<S2Shape> S2GeographyOwningPoint::Shape(int id) const {
    return absl::make_unique<S2PointVectorShape>(points_);
}

std::unique_ptr<S2Region> S2GeographyOwningPoint::Region() const {
    auto region = absl::make_unique<S2RegionUnion>();
    for (const S2Point& point: points_) {
        region->Add(absl::make_unique<S2PointRegion>(point));
    }

    // because Rtools for R 3.6 on Windows complains about a direct
    // return region
    return std::unique_ptr<S2Region>(region.release());
}

void S2GeographyOwningPoint::GetCellUnionBound(std::vector<S2CellId>* cell_ids) const {
    if (points_.size() < 10) {
        for (const S2Point& point: points_) {
            cell_ids->push_back(S2CellId(point));
        }
    } else {
        S2Geography::GetCellUnionBound(cell_ids);
    }
}


int S2GeographyOwningPolyline::num_shapes() const { return polylines_.size(); }

std::unique_ptr<S2Shape> S2GeographyOwningPolyline::Shape(int id) const {
    return absl::make_unique<S2Polyline::Shape>(polylines_[id].get());
}

std::unique_ptr<S2Region> S2GeographyOwningPolyline::Region() const {
    auto region = absl::make_unique<S2RegionUnion>();
    for (const auto& polyline: polylines_) {
        region->Add(std::unique_ptr<S2Region>(polyline->Clone()));
    }
    // because Rtools for R 3.6 on Windows complains about a direct
    // return region
    return std::unique_ptr<S2Region>(region.release());
}

void S2GeographyOwningPolyline::GetCellUnionBound(std::vector<S2CellId>* cell_ids) const {
    for (const auto& polyline: polylines_) {
        polyline->GetCellUnionBound(cell_ids);
    }
}

std::unique_ptr<S2Shape> S2GeographyOwningPolygon::Shape(int id) const {
    return absl::make_unique<S2Polygon::Shape>(polygon_.get());
}

std::unique_ptr<S2Region> S2GeographyOwningPolygon::Region() const {
    return std::unique_ptr<S2Polygon>(polygon_->Clone());
}

void S2GeographyOwningPolygon::GetCellUnionBound(std::vector<S2CellId>* cell_ids) const {
    polygon_->GetCellUnionBound(cell_ids);
}


int S2GeographyCollection::num_shapes() const { return total_shapes_; }

std::unique_ptr<S2Shape> S2GeographyCollection::Shape(int id) const {
    int sum_shapes_ = 0;
    for (int i = 0; i < features_.size(); i++) {
        sum_shapes_ += num_shapes_[i];
        if (id < sum_shapes_) {
            return features_[i]->Shape(id - sum_shapes_ + num_shapes_[i]);
        }
    }

    throw S2GeographyException("shape id out of bounds");
}

std::unique_ptr<S2Region> S2GeographyCollection::Region() const {
    auto region = absl::make_unique<S2RegionUnion>();
    for (const auto& feature: features_) {
        region->Add(feature->Region());
    }
    // because Rtools for R 3.6 on Windows complains about a direct
    // return region
    return std::unique_ptr<S2Region>(region.release());
}
