
#pragma once

#include <stdint.h>
#include <stdexcept>
#include <string>

#include "s2/s2polygon.h"
#include "s2/s2polyline.h"

namespace s2geography {

class S2GeographyException: public std::runtime_error {
public:
    S2GeographyException(std::string what): std::runtime_error(what.c_str()) {}
};

// An S2Geography is an abstraction of S2 types that is designed to closely match
// the scope of a GEOS Geometry. Its methods are limited to those needed to
// implement C API functions. From an S2 perspective, an S2Geography is
// an S2Region that can be represented by zero or more S2Shape objects.
// Current implementations of S2Geography own their data (i.e., the
// coordinate vectors and underlying S2 objects), however, the interface is
// designed to allow future abstractions where this is not the case.
class S2Geography {
public:

    virtual ~S2Geography() {}

    // The number of S2Shape objects needed to represent this S2Geography
    virtual int num_shapes() const = 0;

    // Returns the given S2Shape (where 0 <= id < num_shapes()). The
    // caller retains ownership of the S2Shape but the data pointed to
    // by the object requires that the underlying S2Geography outlives
    // the returned object.
    virtual std::unique_ptr<S2Shape> Shape(int id) const = 0;

    // Returns an S2Region that represents the object. The caller retains
    // ownership of the S2Region but the data pointed to by the object
    // requires that the underlying S2Geography outlives the returned
    // object.
    virtual std::unique_ptr<S2Region> Region() const = 0;

    // Adds an unnormalized set of S2CellIDs to `cell_ids`. This is intended
    // to be faster than using Region().GetCovering() directly and to
    // return a small number of cells that can be used to compute a possible
    // intersection quickly.
    virtual void GetCellUnionBound(std::vector<S2CellId>* cell_ids) const;
};


// An S2Geography representing zero or more points using a std::vector<S2Point>
// as the underlying representation.
class S2GeographyOwningPoint: public S2Geography {
public:
    S2GeographyOwningPoint() {}
    S2GeographyOwningPoint(S2Point point): points_(1) { points_.push_back(point); }
    S2GeographyOwningPoint(std::vector<S2Point> points): points_(std::move(points)) {}

    int num_shapes() const { return 1; }
    std::unique_ptr<S2Shape> Shape(int id) const;
    std::unique_ptr<S2Region> Region() const;
    void GetCellUnionBound(std::vector<S2CellId>* cell_ids) const;

    const std::vector<S2Point>& Points() const {
        return points_;
    }

private:
    std::vector<S2Point> points_;
};


// An S2Geography representing zero or more polylines using the S2Polyline class
// as the underlying representation.
class S2GeographyOwningPolyline: public S2Geography {
public:
    S2GeographyOwningPolyline() {}
    S2GeographyOwningPolyline(std::vector<std::unique_ptr<S2Polyline>> polylines):
        polylines_(std::move(polylines)) {}

    int num_shapes() const;
    std::unique_ptr<S2Shape> Shape(int id) const;
    std::unique_ptr<S2Region> Region() const;
    void GetCellUnionBound(std::vector<S2CellId>* cell_ids) const;

    const std::vector<std::unique_ptr<S2Polyline>>& Polylines() const {
        return polylines_;
    }

private:
    std::vector<std::unique_ptr<S2Polyline>> polylines_;
};


// An S2Geography representing zero or more polygons using the S2Polygon class
// as the underlying representation. Note that a single S2Polygon (from the S2
// perspective) can represent zero or more polygons (from the simple features
// perspective).
class S2GeographyOwningPolygon: public S2Geography {
public:
    S2GeographyOwningPolygon() {}
    S2GeographyOwningPolygon(std::unique_ptr<S2Polygon> polygon):
        polygon_(std::move(polygon)) {}

    int num_shapes() const { return 1; }
    std::unique_ptr<S2Shape> Shape(int id) const;
    std::unique_ptr<S2Region> Region() const;
    void GetCellUnionBound(std::vector<S2CellId>* cell_ids) const;

    const std::unique_ptr<S2Polygon>& Polygon() const {
        return polygon_;
    }

private:
    std::unique_ptr<S2Polygon> polygon_;
};


// An S2Geography wrapping zero or more S2Geography objects. These objects
// can be used to represent a simple features GEOMETRYCOLLECTION.
class S2GeographyCollection: public S2Geography {
public:
    S2GeographyCollection(std::vector<std::unique_ptr<S2Geography>> features):
        features_(std::move(features)),
        total_shapes_(0) {
        for (const auto& feature: features_) {
            num_shapes_.push_back(feature->num_shapes());
            total_shapes_ += feature->num_shapes();
        }
    }

    int num_shapes() const;
    std::unique_ptr<S2Shape> Shape(int id) const;
    std::unique_ptr<S2Region> Region() const;

    const std::vector<std::unique_ptr<S2Geography>>& Features() const {
        return features_;
    }

private:
    std::vector<std::unique_ptr<S2Geography>> features_;
    std::vector<int> num_shapes_;
    int total_shapes_;
};

}
