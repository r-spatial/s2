
#include "geography.hpp"
#include "accessors.hpp"

namespace s2geography {

bool s2_is_collection(const S2Geography& geog) {
    int dimension = s2_dimension(geog);
    if (dimension == 0) {
        return s2_num_points(geog) > 1;
    }

    if (dimension == 1) {
        int num_chains = 0;
        for (int i = 0; i < geog.num_shapes(); i++) {
            std::unique_ptr<S2Shape> shape = geog.Shape(i);
            num_chains += shape->num_chains();
            if (num_chains > 1) {
                return true;
            }
        }

        return false;
    }

    auto polygon_geog_ptr = dynamic_cast<const S2GeographyOwningPolygon*>(&geog);
    if (polygon_geog_ptr != nullptr) {
        int num_outer_loops = 0;
        for (int i = 0; i < polygon_geog_ptr->Polygon()->num_loops(); i++) {
            S2Loop* loop = polygon_geog_ptr->Polygon()->loop(i);
            num_outer_loops += loop->depth() == 0;
            if (num_outer_loops > 1) {
                return true;
            }
        }

        return false;
    } else {
        // if custom subclasses are ever a thing, we can go through the builder
        // to build a polygon
        throw S2GeographyException("s2_area() not implemented for custom S2Geography()");
    }
}

int s2_dimension(const S2Geography& geog) {
    int dimension = -1;
    for (int i = 0; i < geog.num_shapes(); i++) {
        std::unique_ptr<S2Shape> shape = geog.Shape(i);
        if (shape->dimension() > dimension) {
            dimension = shape->dimension();
        }
    }

    return dimension;
}

int s2_num_points(const S2Geography& geog) {
    int num_points = 0;
    for (int i = 0; i < geog.num_shapes(); i++) {
        std::unique_ptr<S2Shape> shape = geog.Shape(i);
        switch (shape->dimension()) {
        case 0:
        case 2:
            num_points += shape->num_edges();
            break;
        case 1:
            num_points += shape->num_edges() - shape->num_chains();
            break;
        }
    }

    return num_points;
}

bool s2_is_empty(const S2Geography& geog) {
    for (int i = 0; i < geog.num_shapes(); i++) {
        std::unique_ptr<S2Shape> shape = geog.Shape(i);
        if (!shape->is_empty()) {
            return false;
        }
    }

    return true;
}

double s2_area(const S2Geography& geog) {
    if (s2_dimension(geog) != 2) {
        return 0;
    }

    auto polygon_geog_ptr = dynamic_cast<const S2GeographyOwningPolygon*>(&geog);
    if (polygon_geog_ptr != nullptr) {
        return polygon_geog_ptr->Polygon()->GetArea();
    } else {
        // if custom subclasses are ever a thing, we can go through the builder
        // to build a polygon
        throw S2GeographyException("s2_area() not implemented for custom S2Geography()");
    }
}

double s2_length(const S2Geography& geog) {
    double length = 0;

    if (s2_dimension(geog) == 1) {
        for (int i = 0; i < geog.num_shapes(); i++) {
            std::unique_ptr<S2Shape> shape = geog.Shape(i);
            for (int j = 0; j < shape->num_edges(); j++) {
                S2Shape::Edge e = shape->edge(j);
                S1ChordAngle angle(e.v0, e.v1);
                length += angle.radians();
            }
        }
    }

    return length;
}

double s2_perimeter(const S2Geography& geog) {
    double length = 0;

    if (s2_dimension(geog) == 2) {
        for (int i = 0; i < geog.num_shapes(); i++) {
            std::unique_ptr<S2Shape> shape = geog.Shape(i);
            for (int j = 0; j < shape->num_edges(); j++) {
                S2Shape::Edge e = shape->edge(j);
                S1ChordAngle angle(e.v0, e.v1);
                length += angle.radians();
            }
        }
    }

    return length;
}

double s2_x(const S2Geography& geog) {
    double out = NAN;
    for (int i = 0; i < geog.num_shapes(); i++) {
        std::unique_ptr<S2Shape> shape = geog.Shape(i);
        if (shape->dimension() == 0 && shape->num_edges() == 1 && std::isnan(out)) {
            out = shape->edge(0).v0.x();
        } else if (shape->dimension() == 0 && shape->num_edges() == 1) {
            return NAN;
        }
    }

    return out;
}

double s2_y(const S2Geography& geog) {
    double out = NAN;
    for (int i = 0; i < geog.num_shapes(); i++) {
        std::unique_ptr<S2Shape> shape = geog.Shape(i);
        if (shape->dimension() == 0 && shape->num_edges() == 1 && std::isnan(out)) {
            out = shape->edge(0).v0.y();
        } else if (shape->dimension() == 0 && shape->num_edges() == 1) {
            return NAN;
        }
    }

    return out;
}

}
