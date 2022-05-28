
#include "s2geography/accessors-geog.h"

#include <s2/s2centroids.h>

#include "s2geography/accessors.h"
#include "s2geography/build.h"
#include "s2geography/geography.h"

namespace s2geography {

S2Point s2_centroid(const Geography& geog) {
  S2Point centroid(0, 0, 0);

  if (geog.dimension() == 0) {
    for (int i = 0; i < geog.num_shapes(); i++) {
      auto shape = geog.Shape(i);
      for (int j = 0; j < shape->num_edges(); j++) {
        centroid += shape->edge(j).v0;
      }
    }

    return centroid.Normalize();
  }

  if (geog.dimension() == 1) {
    for (int i = 0; i < geog.num_shapes(); i++) {
      auto shape = geog.Shape(i);
      for (int j = 0; j < shape->num_edges(); j++) {
        S2Shape::Edge e = shape->edge(j);
        centroid += S2::TrueCentroid(e.v0, e.v1);
      }
    }

    return centroid.Normalize();
  }

  if (geog.dimension() == 2) {
    auto polygon_ptr = dynamic_cast<const PolygonGeography*>(&geog);
    if (polygon_ptr != nullptr) {
      centroid = polygon_ptr->Polygon()->GetCentroid();
    } else {
      std::unique_ptr<PolygonGeography> built = s2_build_polygon(geog);
      centroid = built->Polygon()->GetCentroid();
    }

    return centroid.Normalize();
  }

  auto collection_ptr = dynamic_cast<const GeographyCollection*>(&geog);
  if (collection_ptr == nullptr) {
    throw Exception(
        "Can't compute s2_centroid() on custom collection geography");
  }

  for (auto& feat : collection_ptr->Features()) {
    centroid += s2_centroid(*feat);
  }

  return centroid.Normalize();
}

std::unique_ptr<Geography> s2_boundary(const Geography& geog) {
  int dimension = s2_dimension(geog);

  if (dimension == 1) {
    std::vector<S2Point> endpoints;
    for (int i = 0; i < geog.num_shapes(); i++) {
      auto shape = geog.Shape(i);
      if (shape->dimension() < 1) {
        continue;
      }

      endpoints.reserve(endpoints.size() + shape->num_chains() * 2);
      for (int j = 0; j < shape->num_chains(); j++) {
        S2Shape::Chain chain = shape->chain(j);
        if (chain.length > 0) {
          endpoints.push_back(shape->edge(chain.start).v0);
          endpoints.push_back(shape->edge(chain.start + chain.length - 1).v1);
        }
      }
    }

    return absl::make_unique<PointGeography>(std::move(endpoints));
  }

  if (dimension == 2) {
    std::vector<std::unique_ptr<S2Polyline>> polylines;
    polylines.reserve(geog.num_shapes());

    for (int i = 0; i < geog.num_shapes(); i++) {
      auto shape = geog.Shape(i);
      if (shape->dimension() != 2) {
        throw Exception("Can't extract boundary from heterogeneous collection");
      }

      for (int j = 0; j < shape->num_chains(); j++) {
        S2Shape::Chain chain = shape->chain(j);
        if (chain.length > 0) {
          std::vector<S2Point> points(chain.length + 1);

          points[0] = shape->edge(chain.start).v0;
          for (int k = 0; k < chain.length; k++) {
            points[k + 1] = shape->edge(chain.start + k).v1;
          }

          auto polyline = absl::make_unique<S2Polyline>(std::move(points));
          polylines.push_back(std::move(polyline));
        }
      }
    }

    return absl::make_unique<PolylineGeography>(std::move(polylines));
  }

  return absl::make_unique<GeographyCollection>();
}

std::unique_ptr<Geography> s2_convex_hull(const Geography& geog) {
  S2ConvexHullAggregator agg;
  agg.Add(geog);
  return agg.Finalize();
}

void CentroidAggregator::Add(const Geography& geog) {
  S2Point centroid = s2_centroid(geog);
  if (centroid.Norm2() > 0) {
    centroid_ += centroid.Normalize();
  }
}

void CentroidAggregator::Merge(const CentroidAggregator& other) {
  centroid_ += other.centroid_;
}

S2Point CentroidAggregator::Finalize() {
  if (centroid_.Norm2() > 0) {
    return centroid_.Normalize();
  } else {
    return centroid_;
  }
}

void S2ConvexHullAggregator::Add(const Geography& geog) {
  if (geog.dimension() == 0) {
    auto point_ptr = dynamic_cast<const PointGeography*>(&geog);
    if (point_ptr != nullptr) {
      for (const auto& point : point_ptr->Points()) {
        query_.AddPoint(point);
      }
    } else {
      keep_alive_.push_back(s2_rebuild(geog, GlobalOptions()));
      Add(*keep_alive_.back());
    }

    return;
  }

  if (geog.dimension() == 1) {
    auto poly_ptr = dynamic_cast<const PolylineGeography*>(&geog);
    if (poly_ptr != nullptr) {
      for (const auto& polyline : poly_ptr->Polylines()) {
        query_.AddPolyline(*polyline);
      }
    } else {
      keep_alive_.push_back(s2_rebuild(geog, GlobalOptions()));
      Add(*keep_alive_.back());
    }

    return;
  }

  if (geog.dimension() == 2) {
    auto poly_ptr = dynamic_cast<const PolygonGeography*>(&geog);
    if (poly_ptr != nullptr) {
      query_.AddPolygon(*poly_ptr->Polygon());
    } else {
      keep_alive_.push_back(s2_rebuild(geog, GlobalOptions()));
      Add(*keep_alive_.back());
    }

    return;
  }

  auto collection_ptr = dynamic_cast<const GeographyCollection*>(&geog);
  if (collection_ptr != nullptr) {
    for (const auto& feature : collection_ptr->Features()) {
      Add(*feature);
    }
  } else {
    keep_alive_.push_back(s2_rebuild(geog, GlobalOptions()));
    Add(*keep_alive_.back());
  }
}

std::unique_ptr<PolygonGeography> S2ConvexHullAggregator::Finalize() {
  auto polygon = absl::make_unique<S2Polygon>();
  polygon->Init(query_.GetConvexHull());
  return absl::make_unique<PolygonGeography>(std::move(polygon));
}

}  // namespace s2geography
