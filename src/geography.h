
#ifndef GEOGRAPHY_H
#define GEOGRAPHY_H

#define R_NO_REMAP
#include <R.h>
#include <Rinternals.h>

#include "s2-geography/s2-geography.hpp"

class Geography {
public:
  Geography(std::unique_ptr<s2geography::S2Geography> geog):
    geog_(std::move(geog)), index_(nullptr) {}

  const s2geography::S2Geography& Geog() const {
    return *geog_;
  }

  const s2geography::S2GeographyShapeIndex& Index() {
    if (!index_) {
      this->index_ = absl::make_unique<s2geography::S2GeographyShapeIndex>(*geog_);
    }

    return *index_;
  }

  static SEXP MakeXPtr(std::unique_ptr<s2geography::S2Geography> geog) {
    SEXP xptr = PROTECT(R_MakeExternalPtr(new Geography(std::move(geog)), R_NilValue, R_NilValue));
    R_RegisterCFinalizer(xptr, &finalize_xptr);
    UNPROTECT(1);
    return xptr;
  }

  static SEXP MakeXPtr(std::unique_ptr<Geography> geog) {
    std::unique_ptr<Geography> geog_owning = std::move(geog);
    SEXP xptr = PROTECT(R_MakeExternalPtr(geog_owning.release(), R_NilValue, R_NilValue));
    R_RegisterCFinalizer(xptr, &finalize_xptr);
    UNPROTECT(1);
    return xptr;
  }

  static std::unique_ptr<Geography> MakePoint() {
    return absl::make_unique<Geography>(absl::make_unique<s2geography::S2GeographyOwningPoint>());
  }

  static std::unique_ptr<Geography> MakePoint(S2Point point) {
    return absl::make_unique<Geography>(absl::make_unique<s2geography::S2GeographyOwningPoint>(point));
  }

  static std::unique_ptr<Geography> MakePoint(std::vector<S2Point> points) {
    return absl::make_unique<Geography>(absl::make_unique<s2geography::S2GeographyOwningPoint>(std::move(points)));
  }

  static std::unique_ptr<Geography> MakePolyline() {
    return absl::make_unique<Geography>(absl::make_unique<s2geography::S2GeographyOwningPolyline>());
  }

  static std::unique_ptr<Geography> MakePolyline(std::unique_ptr<S2Polyline> polyline) {
    return absl::make_unique<Geography>(absl::make_unique<s2geography::S2GeographyOwningPolyline>(std::move(polyline)));
  }

  static std::unique_ptr<Geography> MakePolyline(std::vector<std::unique_ptr<S2Polyline>> polylines) {
    return absl::make_unique<Geography>(absl::make_unique<s2geography::S2GeographyOwningPolyline>(std::move(polylines)));
  }

  static std::unique_ptr<Geography> MakePolygon() {
    return absl::make_unique<Geography>(absl::make_unique<s2geography::S2GeographyOwningPolygon>());
  }

  static std::unique_ptr<Geography> MakePolygon(std::unique_ptr<S2Polygon> polygon) {
    return absl::make_unique<Geography>(absl::make_unique<s2geography::S2GeographyOwningPolygon>(std::move(polygon)));
  }

  static std::unique_ptr<Geography> MakeCollection() {
    return absl::make_unique<Geography>(absl::make_unique<s2geography::S2GeographyCollection>());
  }

  static std::unique_ptr<Geography> MakeCollection(std::vector<std::unique_ptr<s2geography::S2Geography>> features) {
    return absl::make_unique<Geography>(absl::make_unique<s2geography::S2GeographyCollection>(std::move(features)));
  }

private:
  std::unique_ptr<s2geography::S2Geography> geog_;
  std::unique_ptr<s2geography::S2GeographyShapeIndex> index_;

  static void finalize_xptr(SEXP xptr) {
    Geography* geog = reinterpret_cast<Geography*>(R_ExternalPtrAddr(xptr));
    if (geog != nullptr) {
      delete geog;
    }
  }
};

#endif
