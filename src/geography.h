
#ifndef GEOGRAPHY_H
#define GEOGRAPHY_H

#include <Rcpp.h>

#include "s2geography.h"

class RGeography {
public:
  RGeography(std::unique_ptr<s2geography::Geography> geog):
    geog_(std::move(geog)), index_(nullptr) {}

  const s2geography::Geography& Geog() const {
    return *geog_;
  }

  const s2geography::ShapeIndexGeography& Index() {
    if (!index_) {
      this->index_ = absl::make_unique<s2geography::ShapeIndexGeography>(*geog_);
    }

    return *index_;
  }

  // For an unknown reason, returning a SEXP from MakeXPtr results in
  // rchk reporting a memory protection error. Until this is sorted, return a
  // Rcpp::XPtr<>() (even though this might be slower)
  static Rcpp::XPtr<RGeography> MakeXPtr(std::unique_ptr<s2geography::Geography> geog) {
    return Rcpp::XPtr<RGeography>(new RGeography(std::move(geog)));
  }

  static Rcpp::XPtr<RGeography> MakeXPtr(std::unique_ptr<RGeography> geog) {
    return Rcpp::XPtr<RGeography>(geog.release());
  }

  static std::unique_ptr<RGeography> MakePoint() {
    return absl::make_unique<RGeography>(absl::make_unique<s2geography::PointGeography>());
  }

  static std::unique_ptr<RGeography> MakePoint(S2Point point) {
    return absl::make_unique<RGeography>(absl::make_unique<s2geography::PointGeography>(point));
  }

  static std::unique_ptr<RGeography> MakePoint(std::vector<S2Point> points) {
    return absl::make_unique<RGeography>(absl::make_unique<s2geography::PointGeography>(std::move(points)));
  }

  static std::unique_ptr<RGeography> MakePolyline() {
    return absl::make_unique<RGeography>(absl::make_unique<s2geography::PolylineGeography>());
  }

  static std::unique_ptr<RGeography> MakePolyline(std::unique_ptr<S2Polyline> polyline) {
    return absl::make_unique<RGeography>(absl::make_unique<s2geography::PolylineGeography>(std::move(polyline)));
  }

  static std::unique_ptr<RGeography> MakePolyline(std::vector<std::unique_ptr<S2Polyline>> polylines) {
    return absl::make_unique<RGeography>(absl::make_unique<s2geography::PolylineGeography>(std::move(polylines)));
  }

  static std::unique_ptr<RGeography> MakePolygon() {
    return absl::make_unique<RGeography>(absl::make_unique<s2geography::PolygonGeography>());
  }

  static std::unique_ptr<RGeography> MakePolygon(std::unique_ptr<S2Polygon> polygon) {
    return absl::make_unique<RGeography>(absl::make_unique<s2geography::PolygonGeography>(std::move(polygon)));
  }

  static std::unique_ptr<RGeography> MakeCollection() {
    return absl::make_unique<RGeography>(absl::make_unique<s2geography::GeographyCollection>());
  }

  static std::unique_ptr<RGeography> MakeCollection(std::vector<std::unique_ptr<s2geography::Geography>> features) {
    return absl::make_unique<RGeography>(absl::make_unique<s2geography::GeographyCollection>(std::move(features)));
  }

private:
  std::unique_ptr<s2geography::Geography> geog_;
  std::unique_ptr<s2geography::ShapeIndexGeography> index_;

  static void finalize_xptr(SEXP xptr) {
    RGeography* geog = reinterpret_cast<RGeography*>(R_ExternalPtrAddr(xptr));
    if (geog != nullptr) {
      delete geog;
    }
  }
};

#endif
