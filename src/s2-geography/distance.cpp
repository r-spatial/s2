
#include "s2/s2closest_edge_query.h"
#include "s2/s2furthest_edge_query.h"

#include "geography.hpp"
#include "distance.hpp"

namespace s2geography {

double s2_distance(const ShapeIndexGeography& geog1, const ShapeIndexGeography& geog2) {
    S2ClosestEdgeQuery query(&geog1.ShapeIndex());
    S2ClosestEdgeQuery::ShapeIndexTarget target(&geog2.ShapeIndex());

    const auto& result = query.FindClosestEdge(&target);

    S1ChordAngle angle = result.distance();
    return angle.ToAngle().radians();
}

double s2_max_distance(const ShapeIndexGeography& geog1, const ShapeIndexGeography& geog2) {
    S2FurthestEdgeQuery query(&geog1.ShapeIndex());
    S2FurthestEdgeQuery::ShapeIndexTarget target(&geog2.ShapeIndex());

    const auto& result = query.FindFurthestEdge(&target);

    S1ChordAngle angle = result.distance();
    return angle.ToAngle().radians();
}

S2Point s2_closest_point(const ShapeIndexGeography& geog1, const ShapeIndexGeography& geog2) {
    return s2_minimum_clearance_line_between(geog1, geog2).first;
}

std::pair<S2Point, S2Point> s2_minimum_clearance_line_between(
    const ShapeIndexGeography& geog1, const ShapeIndexGeography& geog2) {
    S2ClosestEdgeQuery query1(&geog1.ShapeIndex());
    query1.mutable_options()->set_include_interiors(false);
    S2ClosestEdgeQuery::ShapeIndexTarget target(&geog2.ShapeIndex());

    const auto& result1 = query1.FindClosestEdge(&target);

    if (result1.edge_id() == -1) {
        return std::pair<S2Point, S2Point>(S2Point(0, 0, 0), S2Point(0, 0, 0));
    }

    // Get the edge from index1 (edge1) that is closest to index2.
    S2Shape::Edge edge1 = query1.GetEdge(result1);

    // Now find the edge from index2 (edge2) that is closest to edge1.
    S2ClosestEdgeQuery query2(&geog2.ShapeIndex());
    query2.mutable_options()->set_include_interiors(false);
    S2ClosestEdgeQuery::EdgeTarget target2(edge1.v0, edge1.v1);
    auto result2 = query2.FindClosestEdge(&target2);

    // what if result2 has no edges?
    if (result2.is_interior()) {
        throw Exception("S2ClosestEdgeQuery result is interior!");
    }

    S2Shape::Edge edge2 = query2.GetEdge(result2);

    // Find the closest point pair on edge1 and edge2.
    return S2::GetEdgePairClosestPoints(
        edge1.v0, edge1.v1,
        edge2.v0, edge2.v1
    );
}

}
