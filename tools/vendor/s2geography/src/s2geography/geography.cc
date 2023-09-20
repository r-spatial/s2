
#include "s2geography/geography.h"

#include <s2/mutable_s2shape_index.h>
#include <s2/s2point_region.h>
#include <s2/s2point_vector_shape.h>
#include <s2/s2polygon.h>
#include <s2/s2polyline.h>
#include <s2/s2region.h>
#include <s2/s2region_union.h>
#include <s2/s2shape.h>
#include <s2/s2shape_index_region.h>
#include <s2/s2shapeutil_coding.h>

using namespace s2geography;

// This class is a shim to allow a class to return a std::unique_ptr<S2Shape>(),
// which is required by MutableS2ShapeIndex::Add(), without copying the
// underlying data. S2Shape instances do not typically own their data (e.g.,
// S2Polygon::Shape), so this does not change the general relationship (that
// anything returned by Geography::Shape() is only valid within the scope of
// the Geography). Note that this class is also available (but not exposed) in
// s2/s2shapeutil_coding.cc.
class S2ShapeWrapper : public S2Shape {
 public:
  S2ShapeWrapper(S2Shape* shape) : shape_(shape) {}
  int num_edges() const { return shape_->num_edges(); }
  Edge edge(int edge_id) const { return shape_->edge(edge_id); }
  int dimension() const { return shape_->dimension(); }
  ReferencePoint GetReferencePoint() const {
    return shape_->GetReferencePoint();
  }
  int num_chains() const { return shape_->num_chains(); }
  Chain chain(int chain_id) const { return shape_->chain(chain_id); }
  Edge chain_edge(int chain_id, int offset) const {
    return shape_->chain_edge(chain_id, offset);
  }
  ChainPosition chain_position(int edge_id) const {
    return shape_->chain_position(edge_id);
  }

 private:
  S2Shape* shape_;
};

// Just like the S2ShapeWrapper, the S2RegionWrapper helps reconcile the
// differences in lifecycle expectation between S2 and Geography. We often
// need access to a S2Region to generalize algorithms; however, there are some
// operations that need ownership of the region (e.g., the S2RegionUnion). In
// Geography the assumption is that anything returned by a Geography is only
// valid for the lifetime of the underlying Geography. A different design of
// the algorithms implemented here might well make this unnecessary.
class S2RegionWrapper : public S2Region {
 public:
  S2RegionWrapper(S2Region* region) : region_(region) {}
  S2Region* Clone() const { return region_->Clone(); }
  S2Cap GetCapBound() const { return region_->GetCapBound(); }
  S2LatLngRect GetRectBound() const { return region_->GetRectBound(); }
  void GetCellUnionBound(std::vector<S2CellId>* cell_ids) const {
    return region_->GetCellUnionBound(cell_ids);
  }
  bool Contains(const S2Cell& cell) const { return region_->Contains(cell); }
  bool MayIntersect(const S2Cell& cell) const {
    return region_->MayIntersect(cell);
  }
  bool Contains(const S2Point& p) const { return region_->Contains(p); }

 private:
  S2Region* region_;
};

void Geography::GetCellUnionBound(std::vector<S2CellId>* cell_ids) const {
  MutableS2ShapeIndex index;
  for (int i = 0; i < num_shapes(); i++) {
    index.Add(Shape(i));
  }

  MakeS2ShapeIndexRegion<MutableS2ShapeIndex>(&index).GetCellUnionBound(
      cell_ids);
}

std::unique_ptr<S2Shape> PointGeography::Shape(int /*id*/) const {
  return absl::make_unique<S2PointVectorShape>(points_);
}

std::unique_ptr<S2Region> PointGeography::Region() const {
  auto region = absl::make_unique<S2RegionUnion>();
  for (const S2Point& point : points_) {
    region->Add(absl::make_unique<S2PointRegion>(point));
  }

  // because Rtools for R 3.6 on Windows complains about a direct
  // return region
  return std::unique_ptr<S2Region>(region.release());
}

void PointGeography::GetCellUnionBound(std::vector<S2CellId>* cell_ids) const {
  if (points_.size() < 10) {
    for (const S2Point& point : points_) {
      cell_ids->push_back(S2CellId(point));
    }
  } else {
    Geography::GetCellUnionBound(cell_ids);
  }
}

int PolylineGeography::num_shapes() const {
  return static_cast<int>(polylines_.size());
}

std::unique_ptr<S2Shape> PolylineGeography::Shape(int id) const {
  return absl::make_unique<S2Polyline::Shape>(polylines_[id].get());
}

std::unique_ptr<S2Region> PolylineGeography::Region() const {
  auto region = absl::make_unique<S2RegionUnion>();
  for (const auto& polyline : polylines_) {
    region->Add(absl::make_unique<S2RegionWrapper>(polyline.get()));
  }
  // because Rtools for R 3.6 on Windows complains about a direct
  // return region
  return std::unique_ptr<S2Region>(region.release());
}

void PolylineGeography::GetCellUnionBound(
    std::vector<S2CellId>* cell_ids) const {
  for (const auto& polyline : polylines_) {
    polyline->GetCellUnionBound(cell_ids);
  }
}

std::unique_ptr<S2Shape> PolygonGeography::Shape(int /*id*/) const {
  return absl::make_unique<S2Polygon::Shape>(polygon_.get());
}

std::unique_ptr<S2Region> PolygonGeography::Region() const {
  return absl::make_unique<S2RegionWrapper>(polygon_.get());
}

void PolygonGeography::GetCellUnionBound(
    std::vector<S2CellId>* cell_ids) const {
  polygon_->GetCellUnionBound(cell_ids);
}

int GeographyCollection::num_shapes() const { return total_shapes_; }

std::unique_ptr<S2Shape> GeographyCollection::Shape(int id) const {
  int sum_shapes_ = 0;
  for (int i = 0; i < static_cast<int>(features_.size()); i++) {
    sum_shapes_ += num_shapes_[i];
    if (id < sum_shapes_) {
      return features_[i]->Shape(id - sum_shapes_ + num_shapes_[i]);
    }
  }

  throw Exception("shape id out of bounds");
}

std::unique_ptr<S2Region> GeographyCollection::Region() const {
  auto region = absl::make_unique<S2RegionUnion>();
  for (const auto& feature : features_) {
    region->Add(feature->Region());
  }
  // because Rtools for R 3.6 on Windows complains about a direct
  // return region
  return std::unique_ptr<S2Region>(region.release());
}

int ShapeIndexGeography::num_shapes() const {
  return shape_index_.num_shape_ids();
}

std::unique_ptr<S2Shape> ShapeIndexGeography::Shape(int id) const {
  S2Shape* shape = shape_index_.shape(id);
  return std::unique_ptr<S2Shape>(new S2ShapeWrapper(shape));
}

std::unique_ptr<S2Region> ShapeIndexGeography::Region() const {
  auto region =
      absl::make_unique<S2ShapeIndexRegion<MutableS2ShapeIndex>>(&shape_index_);
  // because Rtools for R 3.6 on Windows complains about a direct
  // return region
  return std::unique_ptr<S2Region>(region.release());
}
