
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


// This class is a shim to allow a class to return a std::unique_ptr<S2Shape>(),
// which is required by MutableS2ShapeIndex::Add(), without copying the underlying
// data. S2Shape instances do not typically own their data (e.g., S2Polygon::Shape),
// so this does not change the general relationship (that anything returned by
// S2Geography::Shape() is only valid within the scope of the S2Geography).
class S2ShapeWrapper: public S2Shape {
public:
    S2ShapeWrapper(S2Shape* shape): shape_(shape) {}
    int num_edges() const { return shape_->num_edges();}
    Edge edge(int edge_id) const { return shape_->edge(edge_id); }
    int dimension() const { return shape_->dimension(); }
    ReferencePoint GetReferencePoint() const { return shape_->GetReferencePoint(); }
    int num_chains() const { return shape_->num_chains(); }
    Chain chain(int chain_id) const { return shape_->chain(chain_id); }
    Edge chain_edge(int chain_id, int offset) const { return shape_->chain_edge(chain_id, offset); }
    ChainPosition chain_position(int edge_id) const { return shape_->chain_position(edge_id); }

private:
    S2Shape* shape_;
};


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

int S2GeographyShapeIndex::num_shapes() const { return shape_index_.num_shape_ids(); }

std::unique_ptr<S2Shape> S2GeographyShapeIndex::Shape(int id) const {
    S2Shape* shape = shape_index_.shape(id);
    return std::unique_ptr<S2Shape>(new S2ShapeWrapper(shape));
}

std::unique_ptr<S2Region> S2GeographyShapeIndex::Region() const {
    auto region = absl::make_unique<S2ShapeIndexRegion<MutableS2ShapeIndex>>(&shape_index_);
    // because Rtools for R 3.6 on Windows complains about a direct
    // return region
    return std::unique_ptr<S2Region>(region.release());
}