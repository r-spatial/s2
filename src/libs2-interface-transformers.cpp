
#include "libs2-s2geography.h"
#include "libs2-geography-operator.h"
#include "s2/s2boolean_operation.h"
#include "s2/s2closest_edge_query.h"
#include "s2/s2polygon.h"
#include "s2/s2polyline.h"
#include "s2/s2point.h"
#include "s2/s2error.h"
#include "s2/s2builderutil_s2polygon_layer.h"
#include "s2/s2builderutil_s2polyline_vector_layer.h"
#include "s2/s2builderutil_s2point_vector_layer.h"

#include <Rcpp.h>
using namespace Rcpp;

// [[Rcpp::export]]
List libs2_cpp_s2_intersection(List geog1, List geog2) {
  class LibS2Op: public LibS2BinaryGeographyOperator<List, SEXP> {

    SEXP processFeature(XPtr<LibS2Geography> feature1, XPtr<LibS2Geography> feature2, R_xlen_t i) {

      std::vector<S2Point> points;
      std::vector<std::unique_ptr<S2Polyline>> polylines;
      S2Polygon polygon;

      std::vector<std::unique_ptr<S2Builder::Layer>> layers;
      layers.push_back(absl::make_unique<s2builderutil::S2PointVectorLayer>(&points));
      layers.push_back(absl::make_unique<s2builderutil::S2PolylineVectorLayer>(&polylines));
      layers.push_back(absl::make_unique<s2builderutil::S2PolygonLayer>(&polygon));

      S2BooleanOperation op(S2BooleanOperation::OpType::INTERSECTION, std::move(layers));

      S2Error error;
      if (!op.Build(*feature1->ShapeIndex(), *feature2->ShapeIndex(), &error)) {
        stop(error.text());
      }

      if (polylines.size() > 0) {
        stop("Can't handle polyline output (yet)");
      }

      if (!polygon.is_empty()) {
        stop("Can't handle polygon output (yet)");
      }

      if (points.size() == 0) {
        return XPtr<LibS2Geography>(new LibS2PointGeography());
      } else if (points.size() == 1) {
        return XPtr<LibS2Geography>(new LibS2PointGeography(S2LatLng(points[0])));
      } else {
        stop("Can't handle multipoint output (yet)");
      }

      return R_NilValue;
    }
  };

  LibS2Op op;
  return op.processVector(geog1, geog2);
}
