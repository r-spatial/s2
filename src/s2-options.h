
#ifndef S2_OPTIONS_H
#define S2_OPTIONS_H

#include <sstream>
#include <Rcpp.h>
#include "s2/s2boolean_operation.h"
#include "s2/s2builderutil_snap_functions.h"
#include "s2/s2builderutil_s2polygon_layer.h"
#include "s2/s2builderutil_s2polyline_vector_layer.h"
#include "s2/s2builderutil_s2point_vector_layer.h"

// This class wraps several concepts in the S2BooleanOperation,
// and S2Layer, parameterized such that these can be specified from R
class GeographyOperationOptions {
public:
  int polygonModel;
  int polylineModel;
  Rcpp::List snap;
  double snapRadius;
  int duplicatePointEdges;
  int duplicatePolylineEdges;
  int duplicatePolygonEdges;
  int polylineEdgeType;
  int polygonEdgeType;
  int validatePolyline;
  int validatePolygon;
  int polylineType;
  int polylineSiblingPairs;
  int simplifyEdgeChains;
  int splitCrossingEdges;
  int idempotent;
  int dimensions;

  enum Dimension {
    POINT = 1,
    POLYLINE = 2,
    POLYGON = 4
  };

  // Wraps options for the three layer types
  class LayerOptions {
    public:
    s2builderutil::S2PointVectorLayer::Options pointLayerOptions;
    s2builderutil::S2PolylineVectorLayer::Options polylineLayerOptions;
    s2builderutil::S2PolygonLayer::Options polygonLayerOptions;
    int dimensions;
  };

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
      int model = s2options["model"];
      this->polylineModel = model;
      this->polygonModel = model;
    } catch (std::exception& e) {
      std::stringstream err;
      err << "Error setting s2_options() `model`: " << e.what();
      Rcpp::stop(err.str());
    }

    try {
      this->snap = s2options["snap"];
    } catch (std::exception& e) {
      std::stringstream err;
      err << "Error setting s2_options() `snap`: " << e.what();
      Rcpp::stop(err.str());
    }

    try {
      this->snapRadius = s2options["snap_radius"];
    } catch (std::exception& e) {
      std::stringstream err;
      err << "Error setting s2_options() `snap_radius`: " << e.what();
      Rcpp::stop(err.str());
    }

    try {
      int duplicateEdges = s2options["duplicate_edges"];
      this->duplicatePointEdges = duplicateEdges;
      this->duplicatePolylineEdges = duplicateEdges;
      this->duplicatePolygonEdges = duplicateEdges;
    } catch (std::exception& e) {
      std::stringstream err;
      err << "Error setting s2_options() `duplicate_edges`: " << e.what();
      Rcpp::stop(err.str());
    }

    try {
      int edgeType = s2options["edge_type"];
      this->polylineEdgeType = edgeType;
      this->polygonEdgeType = edgeType;
    } catch (std::exception& e) {
      std::stringstream err;
      err << "Error setting s2_options() `edge_type`: " << e.what();
      Rcpp::stop(err.str());
    }

    try {
      int validate = s2options["validate"];
      this->validatePolyline = validate;
      this->validatePolygon = validate;
    } catch (std::exception& e) {
      std::stringstream err;
      err << "Error setting s2_options() `duplicate_edges`: " << e.what();
      Rcpp::stop(err.str());
    }

    try {
      this->polylineType = s2options["polyline_type"];
    } catch (std::exception& e) {
      std::stringstream err;
      err << "Error setting s2_options() `polyline_type`: " << e.what();
      Rcpp::stop(err.str());
    }

    try {
      this->polylineSiblingPairs = s2options["polyline_sibling_pairs"];
    } catch (std::exception& e) {
      std::stringstream err;
      err << "Error setting s2_options() `polyline_sibling_pairs`: " << e.what();
      Rcpp::stop(err.str());
    }

    try {
      this->simplifyEdgeChains = s2options["simplify_edge_chains"];
    } catch (std::exception& e) {
      std::stringstream err;
      err << "Error setting s2_options() `simplify_edge_chains`: " << e.what();
      Rcpp::stop(err.str());
    }

    try {
      this->splitCrossingEdges = s2options["split_crossing_edges"];
    } catch (std::exception& e) {
      std::stringstream err;
      err << "Error setting s2_options() `split_crossing_edges`: " << e.what();
      Rcpp::stop(err.str());
    }

    try {
      this->idempotent = s2options["idempotent"];
    } catch (std::exception& e) {
      std::stringstream err;
      err << "Error setting s2_options() `idempotent`: " << e.what();
      Rcpp::stop(err.str());
    }

    try {
      this->dimensions = 0;
      Rcpp::IntegerVector dim = s2options["dimensions"];
      for (int i = 0; i < dim.size(); i++) {
        switch (dim[i]) {
        case 1:
          this->dimensions |= Dimension::POINT;
          break;
        case 2:
          this->dimensions |= Dimension::POLYLINE;
          break;
        case 3:
          this->dimensions |= Dimension::POLYGON;
          break;
        }
      }
    } catch (std::exception& e) {
      std::stringstream err;
      err << "Error setting s2_options() `dimensions`: " << e.what();
      Rcpp::stop(err.str());
    }
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
    this->setSnapFunction<S2BooleanOperation::Options>(options);

    return options;
  }

  // build options for S2Builder
  S2Builder::Options builderOptions() {
    S2Builder::Options options;
    options.set_simplify_edge_chains(this->simplifyEdgeChains);
    options.set_split_crossing_edges(this->splitCrossingEdges);
    options.set_idempotent(this->idempotent);
    this->setSnapFunction<S2Builder::Options>(options);
    return options;
  }

  // build options for point, polyline, and polygon layers
  LayerOptions layerOptions() {
    LayerOptions out;

    // point layer
    out.pointLayerOptions.set_duplicate_edges(getDuplicateEdges(this->duplicatePointEdges));

    // polyline layer
    out.polylineLayerOptions.set_duplicate_edges(getDuplicateEdges(this->duplicatePolylineEdges));
    out.polylineLayerOptions.set_edge_type(getEdgeType(this->polylineEdgeType));
    out.polylineLayerOptions.set_polyline_type(getPolylineType(this->polylineType));
    out.polylineLayerOptions.set_sibling_pairs(getSiblingPairs(this->polylineSiblingPairs));
    out.polylineLayerOptions.set_validate(this->validatePolyline);

    // always disable debugging where possible
    out.polylineLayerOptions.set_s2debug_override(S2Debug::DISABLE);

    // polygon layer
    out.polygonLayerOptions.set_edge_type(getEdgeType(this->polygonEdgeType));
    out.polygonLayerOptions.set_validate(this->validatePolygon);

    // dimensions
    out.dimensions = this->dimensions;

    return out;
  }

  template <class OptionsType>
  void setSnapFunction(OptionsType& options) {
    // S2Builder::SnapFunction is abstract and can't be returned
    // hence the templated function

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
  }

  static S2BooleanOperation::PolygonModel getPolygonModel(int model) {
    switch (model) {
      case 1: return S2BooleanOperation::PolygonModel::OPEN;
      case 2: return S2BooleanOperation::PolygonModel::SEMI_OPEN;
      case 3: return S2BooleanOperation::PolygonModel::CLOSED;
      default:
        std::stringstream err;
        err << "Invalid value for polygon model: " << model;
        Rcpp::stop(err.str());
    }
  }

  static S2BooleanOperation::PolylineModel getPolylineModel(int model) {
    switch (model) {
      case 1: return S2BooleanOperation::PolylineModel::OPEN;
      case 2: return S2BooleanOperation::PolylineModel::SEMI_OPEN;
      case 3: return S2BooleanOperation::PolylineModel::CLOSED;
      default:
        std::stringstream err;
        err << "Invalid value for polyline model: " << model;
        Rcpp::stop(err.str());
    }
  }

  static S2Builder::GraphOptions::DuplicateEdges getDuplicateEdges(int value) {
    switch (value) {
      case 0: return S2Builder::GraphOptions::DuplicateEdges::MERGE;
      case 1: return S2Builder::GraphOptions::DuplicateEdges::KEEP;
      default:
        std::stringstream err;
        err << "Invalid value for duplicate edges: " << value;
        Rcpp::stop(err.str());
    }
  }

  static S2Builder::GraphOptions::EdgeType getEdgeType(int value) {
    switch (value) {
      case 1: return S2Builder::GraphOptions::EdgeType::DIRECTED;
      case 2: return S2Builder::GraphOptions::EdgeType::UNDIRECTED;
      default:
        std::stringstream err;
        err << "Invalid value for edge type: " << value;
        Rcpp::stop(err.str());
    }
  }

  static S2Builder::GraphOptions::SiblingPairs getSiblingPairs(int value) {
    switch (value) {
      case 1: return S2Builder::GraphOptions::SiblingPairs::DISCARD;
      case 2: return S2Builder::GraphOptions::SiblingPairs::KEEP;
      default:
        std::stringstream err;
        err << "Invalid value for sibling pairs: " << value;
        Rcpp::stop(err.str());
    }
  }

  static S2Builder::Graph::PolylineType getPolylineType(int value) {
    switch (value) {
      case 1: return S2Builder::Graph::PolylineType::PATH;
      case 2: return S2Builder::Graph::PolylineType::WALK;
      default:
        std::stringstream err;
        err << "Invalid value for polylie type: " << value;
        Rcpp::stop(err.str());
    }
  }
};

#endif
