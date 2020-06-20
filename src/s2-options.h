
#include <sstream>
#include <Rcpp.h>
#include "s2/s2boolean_operation.h"
#include "s2/s2builderutil_snap_functions.h"

// This class wraps several concepts in the S2BooleanOperation,
// and S2Layer, parameterized such that these can be specified from R
class GeographyOperationOptions {
public:
  int polygonModel;
  int polylineModel;
  Rcpp::List snap;
  double snapRadius;

  // deaults: use S2 defaults
  GeographyOperationOptions(): polygonModel(-1), polylineModel(-1), snapRadius(-1) {
    this->snap.attr("class") = "snap_identity";
  }

  // create from s2_options() object
  GeographyOperationOptions(Rcpp::List s2options): GeographyOperationOptions() {
    if (!Rf_inherits(s2options, "s2_options")) {
      Rcpp::stop("`options` must be created using s2_options()");
    }

    // if these items are of an incorrect type (e.g., list() instead of int)
    // the default errors are very difficult to diagnose.
    try {
      this->setPolygonModel(s2options["polygon_model"]);
    } catch (std::exception& e) {
      std::stringstream err;
      err << "Error setting s2_options() `polygon_model`: " << e.what();
      Rcpp::stop(err.str());
    }

    try {
      this->setPolylineModel(s2options["polyline_model"]);
    } catch (std::exception& e) {
      std::stringstream err;
      err << "Error setting s2_options() `polyline_model`: " << e.what();
      Rcpp::stop(err.str());
    }

    try {
      this->setSnap(s2options["snap"]);
    } catch (std::exception& e) {
      std::stringstream err;
      err << "Error setting s2_options() `snap`: " << e.what();
      Rcpp::stop(err.str());
    }

    try {
      this->setSnapRadius(s2options["snap_radius"]);
    } catch (std::exception& e) {
      std::stringstream err;
      err << "Error setting s2_options() `snap_radius`: " << e.what();
      Rcpp::stop(err.str());
    }
  }

  // 0 = open, 1 = semi_open, 2 = closed
  void setPolygonModel(int model) {
    this->polygonModel = model;
  }

  // 0 = open, 1 = semi_open, 2 = closed
  void setPolylineModel(int model) {
    this->polylineModel = model;
  }

  void setSnap(Rcpp::List snap) {
    this->snap = snap;
  }

  void setSnapRadius(double snapRadius) {
    this->snapRadius = snapRadius;
  }

  // build options for passing this to the S2BooleanOperation
  S2BooleanOperation::Options booleanOperationOptions() {
    S2BooleanOperation::Options options;
    if (this->polygonModel >= 0) {
      options.set_polygon_model(getPolygonModel(this->polygonModel));
    }

    if (this->polylineModel >= 0) {
      options.set_polyline_model(getPolylineModel(this->polylineModel));
    }

    // setting the snap function and radius here instead of in a function because
    // S2Builder::SnapFunction is abstract and can't be returned
    // there must be a cleaner way to do this

    if (Rf_inherits(this->snap, "snap_identity")) {
      s2builderutil::IdentitySnapFunction snapFunction;
      if (this->snapRadius > 0) {
        snapFunction.set_snap_radius(S1Angle::Radians(this->snapRadius));
      }
      options.set_snap_function(snapFunction);

    } else if (Rf_inherits(this->snap, "snap_level")) {
      int snapLevel = this->snap["level"];
      s2builderutil::S2CellIdSnapFunction snapFunction(snapLevel);
      if (this->snapRadius > 0) {
        snapFunction.set_snap_radius(S1Angle::Radians(this->snapRadius));
      }
      options.set_snap_function(snapFunction);

    } else if (Rf_inherits(this->snap, "snap_precision")) {
      int exponent = snap["exponent"];
      s2builderutil::IntLatLngSnapFunction snapFunction(exponent);
      if (this->snapRadius > 0) {
        snapFunction.set_snap_radius(S1Angle::Radians(this->snapRadius));
      }
      options.set_snap_function(snapFunction);

    } else if (Rf_inherits(this->snap, "snap_distance")) {
      double distance = snap["distance"];
      double snapLevel = s2builderutil::S2CellIdSnapFunction::LevelForMaxSnapRadius(
        S1Angle::Radians(distance)
      );
      s2builderutil::S2CellIdSnapFunction snapFunction(snapLevel);
      if (this->snapRadius > 0) {
        snapFunction.set_snap_radius(S1Angle::Radians(this->snapRadius));
      }
      options.set_snap_function(snapFunction);

    } else {
      Rcpp::stop("`snap` must be specified using s2_snap_*()");
    }

    return options;
  }

  static S2BooleanOperation::PolygonModel getPolygonModel(int model) {
    switch (model) {
      case 0: return S2BooleanOperation::PolygonModel::OPEN;
      case 1: return S2BooleanOperation::PolygonModel::SEMI_OPEN;
      case 2: return S2BooleanOperation::PolygonModel::CLOSED;
      default:
        std::stringstream err;
        err << "Invalid value for polygon model: " << model;
        Rcpp::stop(err.str());
    }
  }

  static S2BooleanOperation::PolylineModel getPolylineModel(int model) {
    switch (model) {
      case 0: return S2BooleanOperation::PolylineModel::OPEN;
      case 1: return S2BooleanOperation::PolylineModel::SEMI_OPEN;
      case 2: return S2BooleanOperation::PolylineModel::CLOSED;
      default:
        std::stringstream err;
        err << "Invalid value for polyline model: " << model;
        Rcpp::stop(err.str());
    }
  }
};
