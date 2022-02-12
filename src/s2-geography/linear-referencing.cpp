
#include "geography.hpp"
#include "linear-referencing.hpp"

namespace s2geography {

double s2_project_normalized(const S2GeographyOwningPolyline& geog1,
                             const S2GeographyOwningPoint& geog2) {
    if (geog1.Polylines().size() != 1 || geog2.Points().size() != 1) {
        return NAN;
    }

    S2Point point = geog2.Points()[0];
    int next_vertex;
    S2Point point_on_line = geog1.Polylines()[0]->Project(point, &next_vertex);
    return geog1.Polylines()[0]->UnInterpolate(point_on_line, next_vertex);
}

double s2_project_normalized(const S2Geography& geog1, const S2Geography& geog2) {
    auto geog1_poly_ptr = dynamic_cast<const S2GeographyOwningPolyline*>(&geog1);
    auto geog2_point_ptr = dynamic_cast<const S2GeographyOwningPoint*>(&geog2);
    if (geog1_poly_ptr != nullptr && geog2_point_ptr != nullptr) {
        return s2_project_normalized(*geog1_poly_ptr, *geog2_point_ptr);
    } else {
        return NAN;
    }
}

}
