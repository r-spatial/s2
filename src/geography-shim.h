
#include "point-geography.h"
#include "polyline-geography.h"
#include "polygon-geography.h"
#include "geography-collection.h"
#include "s2-geography/s2-geography.hpp"

std::unique_ptr<Geography> MakeOldGeography(const s2geography::S2Geography& geog) {
    auto point = dynamic_cast<const s2geography::S2GeographyOwningPoint*>(&geog);
    if (point != nullptr) {
        return absl::make_unique<PointGeography>(point->Points());
    }

    auto polyline = dynamic_cast<const s2geography::S2GeographyOwningPolyline*>(&geog);
    if (polyline != nullptr) {
        std::vector<std::unique_ptr<S2Polyline>> polylines;
        for (auto& poly: polyline->Polylines()) {
            polylines.push_back(std::unique_ptr<S2Polyline>(poly->Clone()));
        }

        return absl::make_unique<PolylineGeography>(std::move(polylines));
    }

    auto polygon = dynamic_cast<const s2geography::S2GeographyOwningPolygon*>(&geog);
    if (polygon != nullptr) {
        return absl::make_unique<PolygonGeography>(std::unique_ptr<S2Polygon>(polygon->Polygon()->Clone()));
    }

    auto collection = dynamic_cast<const s2geography::S2GeographyCollection*>(&geog);
    if (collection != nullptr) {
        std::vector<std::unique_ptr<Geography>> features;
        for (auto& feat: collection->Features()) {
            features.push_back(MakeOldGeography(*feat));
        }
        return absl::make_unique<GeographyCollection>(std::move(features));
    }

    throw s2geography::S2GeographyException("Unsupported S2Geography subclass");
}
