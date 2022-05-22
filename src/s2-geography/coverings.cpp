
#include "s2/s2region_coverer.h"
#include "s2/s2shape_index_buffered_region.h"

#include "geography.hpp"
#include "coverings.hpp"
#include "accessors.hpp"
#include "accessors-geog.hpp"

namespace s2geography {

S2Point s2_point_on_surface(const S2Geography& geog, S2RegionCoverer& coverer) {
    if (s2_is_empty(geog)) {
        return S2Point();
    }

    int dimension = s2_dimension(geog);
    if (dimension == 2) {
        std::unique_ptr<S2Region> region = geog.Region();
        S2CellUnion covering = coverer.GetInteriorCovering(*region);

        // Take center of cell with smallest level (biggest)
        int min_level = 31;
        S2Point pt;
        for (const S2CellId& id : covering) {
            if (id.level() < min_level) {
                // Already normalized
                pt = id.ToPoint();
                min_level = id.level();
            }
        }

        return pt;
    }

    if (dimension == 0) {
        // For point, return point closest to centroid
        S2Point centroid = s2_centroid(geog);

        S1Angle nearest_dist = S1Angle::Infinity();
        S1Angle dist;
        S2Point closest_pt;
        for (int i = 0; i < geog.num_shapes(); i++) {
            auto shape = geog.Shape(i);
            for (int j = 0; j < shape->num_edges(); j++) {
                S2Shape::Edge e = shape->edge(j);
                dist = S1Angle(e.v0, centroid);
                if (dist < nearest_dist) {
                    nearest_dist = dist;
                    closest_pt = e.v0;
                }
            }
        }

        return closest_pt;
    }

    throw Exception("s2_point_on_surface() not implemented for polyline");
}

void s2_covering(const S2Geography& geog, std::vector<S2CellId>* covering,
                          S2RegionCoverer& coverer) {
    coverer.GetCovering(*geog.Region(), covering);
}

void s2_interior_covering(const S2Geography& geog, std::vector<S2CellId>* covering,
                          S2RegionCoverer& coverer) {
    coverer.GetInteriorCovering(*geog.Region(), covering);
}

void s2_covering_buffered(const ShapeIndexGeography& geog, double distance_radians,
                          std::vector<S2CellId>* covering,
                          S2RegionCoverer& coverer) {
    S2ShapeIndexBufferedRegion region(&geog.ShapeIndex(), S1ChordAngle::Radians(distance_radians));
    coverer.GetCovering(region, covering);
}

}
