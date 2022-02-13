
#include "s2/s2closest_edge_query.h"
#include "s2/s2furthest_edge_query.h"

#include "geography.hpp"
#include "distance.hpp"

namespace s2geography {

double s2_distance(const S2GeographyShapeIndex& geog1, const S2GeographyShapeIndex& geog2) {
    S2ClosestEdgeQuery query(&geog1.ShapeIndex());
    S2ClosestEdgeQuery::ShapeIndexTarget target(&geog2.ShapeIndex());

    const auto& result = query.FindClosestEdge(&target);

    S1ChordAngle angle = result.distance();
    return angle.ToAngle().radians();
}

double s2_max_distance(const S2GeographyShapeIndex& geog1, const S2GeographyShapeIndex& geog2) {
    S2FurthestEdgeQuery query(&geog1.ShapeIndex());
    S2FurthestEdgeQuery::ShapeIndexTarget target(&geog2.ShapeIndex());

    const auto& result = query.FindFurthestEdge(&target);

    S1ChordAngle angle = result.distance();
    return angle.ToAngle().radians();
}

}
