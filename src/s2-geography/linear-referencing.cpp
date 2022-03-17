
#include "geography.hpp"
#include "linear-referencing.hpp"
#include "build.hpp"
#include "accessors.hpp"

namespace s2geography {

double s2_project_normalized(const S2GeographyOwningPolyline& geog1,
                             const S2Point& point) {
    if (geog1.Polylines().size() != 1 || point.Norm2() == 0) {
        return NAN;
    }

    int next_vertex;
    S2Point point_on_line = geog1.Polylines()[0]->Project(point, &next_vertex);
    return geog1.Polylines()[0]->UnInterpolate(point_on_line, next_vertex);
}

double s2_project_normalized(const S2Geography& geog1, const S2Geography& geog2) {
    if (geog1.dimension() != 1 || geog2.dimension() != 0) {
        return NAN;
    }

    S2Point point;
    for (int i = 0; i < geog2.num_shapes(); i++) {
        auto shape = geog2.Shape(i);
        for (int j = 0; j < shape->num_edges(); j++) {
            if (point.Norm2() != 0) {
                return NAN;
            } else {
                point = shape->edge(j).v0;
            }
        }
    }

    auto geog1_poly_ptr = dynamic_cast<const S2GeographyOwningPolyline*>(&geog1);
    if (geog1_poly_ptr != nullptr) {
        return s2_project_normalized(*geog1_poly_ptr, point);
    }

    std::unique_ptr<S2Geography> geog_poly = s2_rebuild(geog1, S2GeographyOptions());
    return s2_project_normalized(*geog_poly, geog2);
}

S2Point s2_interpolate_normalized(const S2GeographyOwningPolyline& geog, double distance_norm) {
    if (s2_is_empty(geog)) {
        return S2Point();
    } else if (geog.Polylines().size() == 1) {
        return geog.Polylines()[0]->Interpolate(distance_norm);
    } else {
        throw S2GeographyException("`geog` must contain 0 or 1 polyines");
    }
}

S2Point s2_interpolate_normalized(const S2Geography& geog, double distance_norm) {
    if (s2_is_empty(geog)) {
        return S2Point();
    }

    if (geog.dimension() != 1 || geog.num_shapes() > 1) {
        throw S2GeographyException("`geog` must be a single polyline");
    }

    auto geog_poly_ptr = dynamic_cast<const S2GeographyOwningPolyline*>(&geog);
    if (geog_poly_ptr != nullptr) {
        return s2_interpolate_normalized(*geog_poly_ptr, distance_norm);
    }

    std::unique_ptr<S2Geography> geog_poly = s2_rebuild(geog, S2GeographyOptions());
    return s2_interpolate_normalized(*geog_poly, distance_norm);
}

}
