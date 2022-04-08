
# pragma once

#include "geography.h"

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

    virtual ~Constructor() {}

    virtual void geom_start(util::GeometryType geometry_type, int64_t size) {}
    virtual void ring_start(int32_t size) {}

    virtual void coords(const double* coord, int64_t n, int32_t coord_size) {
        for (int64_t i = 0; i < n; i++) {
            S2LatLng pt = S2LatLng::FromDegrees(coord[i * coord_size + 1], coord[i * coord_size]);
            points_.push_back(pt.ToPoint());
        }
    }

    virtual void ring_end() {}
    virtual void geom_end() {}
    virtual std::unique_ptr<S2Geography> finish() = 0;

protected:
    std::vector<S2Point> points_;
};

class PointConstructor: public Constructor {
public:

    void geom_start(util::GeometryType geometry_type, int64_t size) {
        if (size != 0 &&
            geometry_type != util::GeometryType::POINT &&
            geometry_type != util::GeometryType::MULTIPOINT &&
            geometry_type != util::GeometryType::GEOMETRYCOLLECTION) {
            throw S2GeographyException(
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

            S2LatLng pt = S2LatLng::FromDegrees(coord[i * coord_size], coord[i * coord_size + 1]);
            points_.push_back(pt.ToPoint());
        }
    }

    std::unique_ptr<S2Geography> finish() {
        auto result = absl::make_unique<S2GeographyOwningPoint>(std::move(points_));
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
    void geom_start(util::GeometryType geometry_type, int64_t size) {
        if (geometry_type == util::GeometryType::MULTILINESTRING ||
            geometry_type == util::GeometryType::GEOMETRYCOLLECTION) {
            is_linestring_ = false;
        } else if (geometry_type == util::GeometryType::LINESTRING ||
                   size == 0) {
            if (size > 0) {
                points_.reserve(size);
            }
            is_linestring_ = true;
        } else {
            throw S2GeographyException(
                "PolylineConstructor input must be empty, linestring, multilinestring, or collection");
        }
    }

    void geom_end() {
        if (is_linestring_) {
            auto polyline = absl::make_unique<S2Polyline>(std::move(points_));
            polylines_.push_back(std::move(polyline));
            points_.clear();
            is_linestring_ = false;
        }
    }

    std::unique_ptr<S2Geography> finish() {
        auto result = absl::make_unique<S2GeographyOwningPolyline>(std::move(polylines_));
        polylines_.clear();
        return std::unique_ptr<S2Geography>(result.release());
    }

private:
    bool is_linestring_;
    std::vector<std::unique_ptr<S2Polyline>> polylines_;
};

class PolygonConstructor: public Constructor {
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

    PolygonConstructor(const Options& options): options_(options) {}

    void ring_start(int32_t size) {
        loops_.clear();
        if (size > 0) {
            loops_.reserve(size);
        }
    }

    void ring_end() {
        auto loop = absl::make_unique<S2Loop>(std::move(points_));
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
        auto result = absl::make_unique<S2GeographyOwningPolygon>(std::move(polygon));
        return std::unique_ptr<S2Geography>(result.release());
    }

private:
    std::vector<std::unique_ptr<S2Loop>> loops_;
    Options options_;
};

class CollectionConstructor: public Constructor {
public:
    CollectionConstructor(const PolygonConstructor::Options& options = PolygonConstructor::Options()):
        options_(options), level_(0), polygon_constructor_(options),
        collection_constructor_(nullptr) {}

    void geom_start(util::GeometryType geometry_type, int64_t size) {
        level_++;

        if (active_constructor_ != nullptr) {
            active_constructor_->geom_start(geometry_type, size);
            return;
        }

        switch (geometry_type) {
        case util::GeometryType::POINT:
        case util::GeometryType::MULTIPOINT:
            active_constructor_ = &point_constructor_;
            active_constructor_->geom_start(geometry_type, size);
            break;
        case util::GeometryType::LINESTRING:
        case util::GeometryType::MULTILINESTRING:
            active_constructor_ = &polyline_constructor_;
            active_constructor_->geom_start(geometry_type, size);
            break;
        case util::GeometryType::POLYGON:
        case util::GeometryType::MULTIPOLYGON:
            active_constructor_ = &polygon_constructor_;
            active_constructor_->geom_start(geometry_type, size);
            break;
        case util::GeometryType::GEOMETRYCOLLECTION:
            this->collection_constructor_ = absl::make_unique<CollectionConstructor>(options_);
            this->active_constructor_ = this->collection_constructor_.get();
            // don't call geom_start()!
            break;
        default:
            throw S2GeographyException("CollectionConstructor: unsupported geometry type");
        }
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
        if (active_constructor_ != collection_constructor_.get()) {
            active_constructor_->geom_end();
        }

        level_--;
        if (level_ == 0) {
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

    void start_feature() {
        level_ = 0;
        active_constructor_ = nullptr;
    }

    std::unique_ptr<S2Geography> finish_feature() {
        if (features_.empty()) {
            return absl::make_unique<S2GeographyCollection>();
        } else {
            std::unique_ptr<S2Geography> feature = std::move(features_.back());
            features_.pop_back();
            return feature;
        }
    }

private:
    PolygonConstructor::Options options_;
    int level_;
    PointConstructor point_constructor_;
    PolylineConstructor polyline_constructor_;
    PolygonConstructor polygon_constructor_;
    std::unique_ptr<CollectionConstructor> collection_constructor_;
    Constructor* active_constructor_;

    std::vector<std::unique_ptr<S2Geography>> features_;
};

}
