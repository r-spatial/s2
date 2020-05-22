
#include "libs2-geography-operator.h"
#include "s2/s2boolean_operation.h"
#include "s2/s2closest_edge_query.h"
#include "s2/s2latlng_rect.h"
#include "s2/s2polygon.h"
#include "s2/s2testing.h"

#include <Rcpp.h>
using namespace Rcpp;

// [[Rcpp::export]]
LogicalVector libs2_cpp_s2_intersects(List geog1, List geog2) {
  class LibS2Op: public LibS2BinaryGeographyOperator<LogicalVector, int> {

    int processFeature(XPtr<LibS2Geography> feature1, XPtr<LibS2Geography> feature2, R_xlen_t i) {
      return S2BooleanOperation::Intersects(*feature1->ShapeIndex(), *feature2->ShapeIndex());
    }
  };

  LibS2Op op;
  return op.processVector(geog1, geog2);
}

// [[Rcpp::export]]
LogicalVector libs2_cpp_s2_equals(List geog1, List geog2) {
  class LibS2Op: public LibS2BinaryGeographyOperator<LogicalVector, int> {

    int processFeature(XPtr<LibS2Geography> feature1, XPtr<LibS2Geography> feature2, R_xlen_t i) {
      return S2BooleanOperation::Equals(*feature1->ShapeIndex(), *feature2->ShapeIndex());
    }
  };

  LibS2Op op;
  return op.processVector(geog1, geog2);
}

// [[Rcpp::export]]
LogicalVector libs2_cpp_s2_contains(List geog1, List geog2) {
  class LibS2Op: public LibS2BinaryGeographyOperator<LogicalVector, int> {

    int processFeature(XPtr<LibS2Geography> feature1, XPtr<LibS2Geography> feature2, R_xlen_t i) {
      return S2BooleanOperation::Contains(*feature1->ShapeIndex(), *feature2->ShapeIndex());
    }
  };

  LibS2Op op;
  return op.processVector(geog1, geog2);
}

// [[Rcpp::export]]
LogicalVector libs2_cpp_s2_dwithin(List geog1, List geog2, NumericVector distance) {
  if (distance.size() != geog1.size())  {
    stop("Incompatible lengths");
  }

  class LibS2Op: public LibS2BinaryGeographyOperator<LogicalVector, int> {
  public:
    NumericVector distance;
    LibS2Op(NumericVector distance): distance(distance) {}

    int processFeature(XPtr<LibS2Geography> feature1, XPtr<LibS2Geography> feature2, R_xlen_t i) {
      S2ClosestEdgeQuery query(feature1->ShapeIndex());
      S2ClosestEdgeQuery::ShapeIndexTarget target(feature2->ShapeIndex());
      return query.IsDistanceLessOrEqual(&target, S1ChordAngle::Radians(this->distance[i]));
    }
  };

  LibS2Op op(distance);
  return op.processVector(geog1, geog2);
}

// [[Rcpp::export]]
LogicalVector libs2_cpp_s2_intersectsbox(List geog,
                                         NumericVector lng1, NumericVector lat1,
                                         NumericVector lng2, NumericVector lat2,
                                         IntegerVector detail) {

  class LibS2Op: public LibS2UnaryGeographyOperator<LogicalVector, int> {
  public:
    NumericVector lng1, lat1, lng2, lat2;
    IntegerVector detail;

    LibS2Op(NumericVector lng1, NumericVector lat1,
            NumericVector lng2, NumericVector lat2,
            IntegerVector detail):
      lng1(lng1), lat1(lat1), lng2(lng2), lat2(lat2), detail(detail) {}

    int processFeature(XPtr<LibS2Geography> feature, R_xlen_t i) {
      // construct polygon
      // this might be easier with an s2region intersection
      double xmin = this->lng1[i];
      double ymin = this->lat1[i];
      double xmax = this->lng2[i];
      double ymax = this->lat2[i];
      int detail = this->detail[i];
      
      if (detail < 1) {
        stop("Can't create polygon from bounding box with detail < 1");
      }

      // can't just do xmax - xmin because these boxes can wrap around the date line
      S2Point westEquator = S2LatLng::FromDegrees(0, xmin).Normalized().ToPoint();
      S2Point eastEquator = S2LatLng::FromDegrees(0, xmax).Normalized().ToPoint();
      S1ChordAngle width(westEquator, eastEquator);
      double widthDegrees = width.degrees();
      double deltaDegrees = widthDegrees / (double) detail;
      double heightDegrees = ymax - ymin;
      
      // these situations would result in an error below because of
      // duplicate vertices
      if (widthDegrees == 0 || heightDegrees == 0) {
        return false;
      }

      // create polygon vertices
      std::vector<S2Point> points(2 + 2 * detail);
      S2LatLng vertex;

      // south edge
      for (int i = 0; i <= detail; i++) {
        vertex = S2LatLng::FromDegrees(xmin + deltaDegrees * i, ymin).Normalized();
        points[i] = vertex.ToPoint();
      }

      // north edge
      for (int i = 0; i <= detail; i++) {
        vertex = S2LatLng::FromDegrees(xmax - deltaDegrees * i, ymax).Normalized();
        points[detail + 1 + i] = vertex.ToPoint();
      }

      // create polygon
      std::unique_ptr<S2Loop> loop(new S2Loop());
      loop->Init(points);
      std::vector<std::unique_ptr<S2Loop>> loops(1);
      loops[0] = std::move(loop);
      S2Polygon polygon;
      polygon.InitOriented(std::move(loops));
      
      // test intersection
      return S2BooleanOperation::Intersects(polygon.index(), *feature->ShapeIndex());
    }
  };

  LibS2Op op(lng1, lat1, lng2, lat2, detail);
  return op.processVector(geog);
}
