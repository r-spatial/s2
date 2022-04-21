
#pragma once

#include <sstream>

#include "geography.hpp"

namespace s2geography {

namespace util {

enum GeometryType {
    GEOMETRY_TYPE_UNKNOWN = 0,
    POINT = 1,
    LINESTRING = 2,
    POLYGON = 3,
    MULTIPOINT = 4,
    MULTILINESTRING = 5,
    MULTIPOLYGON = 6,
    GEOMETRYCOLLECTION = 7
};

}

class Constructor {
public:

    class Options {
    public:
        Options() : oriented_(false), check_(true) {}
        bool oriented() { return oriented_; }
        void set_oriented(bool oriented) { oriented_ = oriented; }
        bool check() { return check_; }
        void set_check(bool check) { check_ = check; }

    private:
        bool oriented_;
        bool check_;
    };

    Constructor(const Options& options): options_(options) {}

    virtual ~Constructor() {}

    Options* mutable_options() { return &options_; }

    virtual void geom_start(util::GeometryType geometry_type, int64_t size) {}
    virtual void ring_start(int32_t size) {}

    virtual void coords(const double* coord, int64_t n, int32_t coord_size) {
        for (int64_t i = 0; i < n; i++) {
            S2LatLng pt = S2LatLng::FromDegrees(coord[i * coord_size + 1], coord[i * coord_size]);
            points_.push_back(pt.Normalized().ToPoint());
        }
    }

    virtual void ring_end() {}
    virtual void geom_end() {}
    virtual std::unique_ptr<S2Geography> finish() = 0;

protected:
    std::vector<S2Point> points_;
    Options options_;
};

class PointConstructor: public Constructor {
public:

    PointConstructor(): Constructor(Options()) {}

    void geom_start(util::GeometryType geometry_type, int64_t size) {
        if (size != 0 &&
            geometry_type != util::GeometryType::POINT &&
            geometry_type != util::GeometryType::MULTIPOINT &&
            geometry_type != util::GeometryType::GEOMETRYCOLLECTION) {
            throw Exception(
                "PointConstructor input must be empty, point, multipoint, or collection");
        }

        if (size > 0) {
            points_.reserve(points_.size() + size);
        }
    }

    void coords(const double* coord, int64_t n, int32_t coord_size) {
        for (int64_t i = 0; i < n; i++) {
            if (coord_empty(coord + (i * coord_size), coord_size)) {
                continue;
            }

            S2LatLng pt = S2LatLng::FromDegrees(coord[i * coord_size + 1], coord[i * coord_size]);
            points_.push_back(pt.ToPoint());
        }
    }

    std::unique_ptr<S2Geography> finish() {
        auto result = absl::make_unique<PointGeography>(std::move(points_));
        points_.clear();
        return std::unique_ptr<S2Geography>(result.release());
    }

private:
    bool coord_empty(const double* coord, int32_t coord_size) {
        for (int32_t i = 0; i < coord_size; i++) {
            if (!std::isnan(coord[i])) {
                return false;
            }
        }

        return true;
    }
};

class PolylineConstructor: public Constructor {
public:
    PolylineConstructor(const Options& options): Constructor(options) {}

    void geom_start(util::GeometryType geometry_type, int64_t size) {
        if (size != 0 &&
            geometry_type != util::GeometryType::LINESTRING &&
            geometry_type != util::GeometryType::MULTILINESTRING &&
            geometry_type != util::GeometryType::GEOMETRYCOLLECTION) {
            throw Exception(
                "PolylineConstructor input must be empty, linestring, multilinestring, or collection");
        }

        if (size > 0 && geometry_type == util::GeometryType::LINESTRING) {
            points_.reserve(size);
        }
    }

    void geom_end() {
        if (points_.size() > 0) {
            auto polyline = absl::make_unique<S2Polyline>();
            polyline->Init(std::move(points_));

            if (options_.check() && !polyline->IsValid()) {
                polyline->FindValidationError(&error_);
                throw Exception(error_.text());
            }

            polylines_.push_back(std::move(polyline));
            points_.clear();
        }
    }

    std::unique_ptr<S2Geography> finish() {
        std::unique_ptr<PolylineGeography> result;

        if (polylines_.size() > 0) {
            result = absl::make_unique<PolylineGeography>(std::move(polylines_));
            polylines_.clear();
        } else {
            result = absl::make_unique<PolylineGeography>();
        }

        return std::unique_ptr<S2Geography>(result.release());
    }

private:
    std::vector<std::unique_ptr<S2Polyline>> polylines_;
    S2Error error_;
};

class PolygonConstructor: public Constructor {
public:
    PolygonConstructor(const Options& options): Constructor(options) {}

    void ring_start(int32_t size) {
        points_.clear();
        if (size > 0) {
            points_.reserve(size);
        }
    }

    void ring_end() {
        if (points_.size() == 0) {
            return;
        }

        // S2Loop is open instead of closed
        points_.pop_back();
        auto loop = absl::make_unique<S2Loop>();
        loop->set_s2debug_override(S2Debug::DISABLE);
        loop->Init(std::move(points_));

        if (!options_.oriented()) {
            loop->Normalize();
        }

        if (options_.check() && !loop->IsValid()) {
            std::stringstream err;
            err << "Loop " << (loops_.size()) << " is not valid: ";
            loop->FindValidationError(&error_);
            err << error_.text();
            throw Exception(err.str());
        }

        loops_.push_back(std::move(loop));
        points_.clear();
    }

    std::unique_ptr<S2Geography> finish() {
        auto polygon = absl::make_unique<S2Polygon>();
        polygon->set_s2debug_override(S2Debug::DISABLE);
        if (options_.oriented()) {
            polygon->InitOriented(std::move(loops_));
        } else {
            polygon->InitNested(std::move(loops_));
        }

        loops_.clear();

        if (options_.check() && !polygon->IsValid()) {
            polygon->FindValidationError(&error_);
            throw Exception(error_.text());
        }

        auto result = absl::make_unique<PolygonGeography>(std::move(polygon));
        return std::unique_ptr<S2Geography>(result.release());
    }

private:
    std::vector<std::unique_ptr<S2Loop>> loops_;
    S2Error error_;
};

class CollectionConstructor: public Constructor {
public:
    CollectionConstructor(const Options& options):
        Constructor(options),
        polyline_constructor_(options),
        polygon_constructor_(options),
        collection_constructor_(nullptr),
        level_(0) {}

    void geom_start(util::GeometryType geometry_type, int64_t size) {
        level_++;
        if (level_ == 1 && geometry_type == util::GeometryType::GEOMETRYCOLLECTION) {
            active_constructor_ = nullptr;
            return;
        }

        if (active_constructor_ != nullptr) {
            active_constructor_->geom_start(geometry_type, size);
            return;
        }

        switch (geometry_type) {
        case util::GeometryType::POINT:
        case util::GeometryType::MULTIPOINT:
            active_constructor_ = &point_constructor_;
            break;
        case util::GeometryType::LINESTRING:
        case util::GeometryType::MULTILINESTRING:
            active_constructor_ = &polyline_constructor_;
            break;
        case util::GeometryType::POLYGON:
        case util::GeometryType::MULTIPOLYGON:
            active_constructor_ = &polygon_constructor_;
            break;
        case util::GeometryType::GEOMETRYCOLLECTION:
            this->collection_constructor_ = absl::make_unique<CollectionConstructor>(options_);
            this->active_constructor_ = this->collection_constructor_.get();
            break;
        default:
            throw Exception("CollectionConstructor: unsupported geometry type");
        }

        active_constructor_->geom_start(geometry_type, size);
    }

    void ring_start(int32_t size) {
        active_constructor_->ring_start(size);
    }

    void coords(const double* coord, int64_t n, int32_t coord_size) {
        active_constructor_->coords(coord, n, coord_size);
    }

    void ring_end() {
        active_constructor_->ring_end();
    }

    void geom_end() {
        level_--;

        if (level_ >= 1) {
            active_constructor_->geom_end();
        }

        if (level_ == 1) {
            auto feature = active_constructor_->finish();
            features_.push_back(std::move(feature));
            active_constructor_ = nullptr;
        }
    }

    std::unique_ptr<S2Geography> finish() {
        auto result = absl::make_unique<S2GeographyCollection>(std::move(features_));
        features_.clear();
        return std::unique_ptr<S2Geography>(result.release());
    }

private:
    PointConstructor point_constructor_;
    PolylineConstructor polyline_constructor_;
    PolygonConstructor polygon_constructor_;
    std::unique_ptr<CollectionConstructor> collection_constructor_;


protected:
    Constructor* active_constructor_;
    int level_;
    std::vector<std::unique_ptr<S2Geography>> features_;
};

class VectorConstructor: public CollectionConstructor {
public:
    VectorConstructor(const Options& options): CollectionConstructor(options) {}

    void start_feature() {
        active_constructor_ = nullptr;
        level_ = 0;
        features_.clear();
        geom_start(util::GeometryType::GEOMETRYCOLLECTION, 1);
    }

    std::unique_ptr<S2Geography> finish_feature() {
        geom_end();

        if (features_.empty()) {
            return absl::make_unique<S2GeographyCollection>();
        } else {
            std::unique_ptr<S2Geography> feature = std::move(features_.back());
            if (feature.get() == nullptr) {
                throw Exception("finish_feature() generated nullptr");
            }

            features_.pop_back();
            return feature;
        }
    }
};

}
