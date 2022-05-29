
#include "s2/s2shape_index_buffered_region.h"
#include "s2/s2region_coverer.h"

#include "s2-options.h"
#include "geography-operator.h"

#include <Rcpp.h>
using namespace Rcpp;


class BooleanOperationOp: public BinaryGeographyOperator<List, SEXP> {
public:
  BooleanOperationOp(S2BooleanOperation::OpType opType, List s2options):
    opType(opType) {
      GeographyOperationOptions options(s2options);
      this->geography_options = options.geographyOptions();
    }

  SEXP processFeature(XPtr<RGeography> feature1, XPtr<RGeography> feature2, R_xlen_t i) {
    std::unique_ptr<s2geography::Geography> geog_out = s2geography::s2_boolean_operation(
      feature1->Index(), feature2->Index(),
      this->opType,
      this->geography_options);

    return RGeography::MakeXPtr(std::move(geog_out));
  }

private:
  S2BooleanOperation::OpType opType;
  s2geography::GlobalOptions geography_options;
};

// [[Rcpp::export]]
List cpp_s2_intersection(List geog1, List geog2, List s2options) {
  BooleanOperationOp op(S2BooleanOperation::OpType::INTERSECTION, s2options);
  return op.processVector(geog1, geog2);
}

// [[Rcpp::export]]
List cpp_s2_union(List geog1, List geog2, List s2options) {
  BooleanOperationOp op(S2BooleanOperation::OpType::UNION, s2options);
  return op.processVector(geog1, geog2);
}

// [[Rcpp::export]]
List cpp_s2_difference(List geog1, List geog2, List s2options) {
  BooleanOperationOp op(S2BooleanOperation::OpType::DIFFERENCE, s2options);
  return op.processVector(geog1, geog2);
}

// [[Rcpp::export]]
List cpp_s2_sym_difference(List geog1, List geog2, List s2options) {
  BooleanOperationOp op(S2BooleanOperation::OpType::SYMMETRIC_DIFFERENCE, s2options);
  return op.processVector(geog1, geog2);
}

// [[Rcpp::export]]
List cpp_s2_coverage_union_agg(List geog, List s2options, bool naRm) {
  GeographyOperationOptions options(s2options);
  s2geography::S2CoverageUnionAggregator agg(options.geographyOptions());

  SEXP item;
  for (R_xlen_t i = 0; i < geog.size(); i++) {
    item = geog[i];
    if (item == R_NilValue && !naRm) {
      return List::create(R_NilValue);
    }

    if (item != R_NilValue) {
      Rcpp::XPtr<RGeography> feature(item);
      agg.Add(feature->Geog());
    }
  }

  std::unique_ptr<s2geography::Geography> geog_out = agg.Finalize();
  return List::create(RGeography::MakeXPtr(std::move(geog_out)));
}

// [[Rcpp::export]]
List cpp_s2_union_agg(List geog, List s2options, bool naRm) {
  GeographyOperationOptions options(s2options);
  s2geography::S2UnionAggregator agg(options.geographyOptions());

  SEXP item;
  for (R_xlen_t i = 0; i < geog.size(); i++) {
    item = geog[i];
    if (item == R_NilValue && !naRm) {
      return List::create(R_NilValue);
    }

    if (item != R_NilValue) {
      Rcpp::XPtr<RGeography> feature(item);
      agg.Add(feature->Geog());
    }
  }

  std::unique_ptr<s2geography::Geography> geog_out = agg.Finalize();
  return List::create(RGeography::MakeXPtr(std::move(geog_out)));
}

// [[Rcpp::export]]
List cpp_s2_centroid_agg(List geog, bool naRm) {
  s2geography::CentroidAggregator agg;

  SEXP item;
  for (R_xlen_t i = 0; i < geog.size(); i++) {
    item = geog[i];
    if (item == R_NilValue && !naRm) {
      return List::create(R_NilValue);
    }

    if (item != R_NilValue) {
      Rcpp::XPtr<RGeography> feature(item);
      agg.Add(feature->Geog());
    }
  }

  S2Point centroid = agg.Finalize();

  List output(1);
  if (centroid.Norm2() == 0) {
    output[0] = RGeography::MakeXPtr(RGeography::MakePoint());
  } else {
    output[0] = RGeography::MakeXPtr(RGeography::MakePoint(centroid));
  }

  return output;
}

// [[Rcpp::export]]
List cpp_s2_rebuild_agg(List geog, List s2options, bool naRm) {
  GeographyOperationOptions options(s2options);

  s2geography::RebuildAggregator agg(options.geographyOptions());
  std::vector<std::unique_ptr<s2geography::Geography>> geographies;

  SEXP item;
  for (R_xlen_t i = 0; i < geog.size(); i++) {
    item = geog[i];
    if (item == R_NilValue && !naRm) {
      return List::create(R_NilValue);
    }

    if (item != R_NilValue) {
      Rcpp::XPtr<RGeography> feature(item);
      agg.Add(feature->Geog());
    }
  }

  auto geog_out = agg.Finalize();
  return List::create(RGeography::MakeXPtr(std::move(geog_out)));
}

// [[Rcpp::export]]
List cpp_s2_closest_point(List geog1, List geog2) {
  class Op: public BinaryGeographyOperator<List, SEXP> {

    SEXP processFeature(XPtr<RGeography> feature1, XPtr<RGeography> feature2, R_xlen_t i) {
      S2Point pt = s2geography::s2_closest_point(feature1->Index(), feature2->Index());
      if (pt.Norm2() == 0) {
        return RGeography::MakeXPtr(RGeography::MakePoint());
      } else {
        return RGeography::MakeXPtr(RGeography::MakePoint(pt));
      }
    }
  };

  Op op;
  return op.processVector(geog1, geog2);
}

// [[Rcpp::export]]
List cpp_s2_minimum_clearance_line_between(List geog1, List geog2) {
  class Op: public BinaryGeographyOperator<List, SEXP> {

    SEXP processFeature(XPtr<RGeography> feature1, XPtr<RGeography> feature2, R_xlen_t i) {
      std::pair<S2Point, S2Point> pts = s2geography::s2_minimum_clearance_line_between(
        feature1->Index(),
        feature2->Index()
      );

      if (pts.first.Norm2() == 0) {
        return RGeography::MakeXPtr(RGeography::MakePoint());
      }

      std::vector<S2Point> vertices(2);
      vertices[0] = pts.first;
      vertices[1] = pts.second;

      if (pts.first == pts.second) {
        return RGeography::MakeXPtr(RGeography::MakePoint(std::move(vertices)));
      } else {
        std::vector<S2Point> vertices(2);
        vertices[0] = pts.first;
        vertices[1] = pts.second;
        std::unique_ptr<S2Polyline> polyline = absl::make_unique<S2Polyline>();
        polyline->Init(vertices);
        return RGeography::MakeXPtr(RGeography::MakePolyline(std::move(polyline)));
      }
    }
  };

  Op op;
  return op.processVector(geog1, geog2);
}

// [[Rcpp::export]]
List cpp_s2_centroid(List geog) {
  class Op: public UnaryGeographyOperator<List, SEXP> {
    SEXP processFeature(XPtr<RGeography> feature, R_xlen_t i) {
      S2Point centroid = s2geography::s2_centroid(feature->Geog());
      if (centroid.Norm2() == 0) {
        return RGeography::MakeXPtr(RGeography::MakePoint());
      } else {
        return RGeography::MakeXPtr(RGeography::MakePoint(centroid.Normalize()));
      }
    }
  };

  Op op;
  return op.processVector(geog);
}

// [[Rcpp::export]]
List cpp_s2_point_on_surface(List geog) {
  class Op: public UnaryGeographyOperator<List, SEXP> {
  public:
    S2RegionCoverer coverer;

    SEXP processFeature(XPtr<RGeography> feature, R_xlen_t i) {
      S2Point result = s2geography::s2_point_on_surface(feature->Geog(), coverer);
      if (result.Norm2() == 0) {
        return RGeography::MakeXPtr(RGeography::MakePoint());
      } else {
        return RGeography::MakeXPtr(RGeography::MakePoint(result));
      }
    }
  };

  Op op;
  return op.processVector(geog);
}

// [[Rcpp::export]]
List cpp_s2_boundary(List geog) {
  class Op: public UnaryGeographyOperator<List, SEXP> {
    SEXP processFeature(XPtr<RGeography> feature, R_xlen_t i) {
      std::unique_ptr<s2geography::Geography> result = s2geography::s2_boundary(feature->Geog());
      return RGeography::MakeXPtr(std::move(result));
    }
  };

  Op op;
  return op.processVector(geog);
}

// [[Rcpp::export]]
List cpp_s2_rebuild(List geog, List s2options) {
  class Op: public UnaryGeographyOperator<List, SEXP> {
  public:
    Op(List s2options) {
      GeographyOperationOptions options(s2options);
      this->options = options.geographyOptions();
    }

    SEXP processFeature(XPtr<RGeography> feature, R_xlen_t i) {
      std::unique_ptr<s2geography::Geography> ptr = s2geography::s2_rebuild(
        feature->Geog(),
        this->options
      );

      return RGeography::MakeXPtr(std::move(ptr));
    }

  private:
    s2geography::GlobalOptions options;
  };

  Op op(s2options);
  return op.processVector(geog);
}

// [[Rcpp::export]]
List cpp_s2_unary_union(List geog, List s2options) {
  class Op: public UnaryGeographyOperator<List, SEXP> {
  public:
    Op(List s2options) {
      GeographyOperationOptions options(s2options);
      this->geographyOptions = options.geographyOptions();
    }

    SEXP processFeature(XPtr<RGeography> feature, R_xlen_t i) {
      std::unique_ptr<s2geography::Geography> geog_out =
        s2geography::s2_unary_union(feature->Index(), this->geographyOptions);
      return RGeography::MakeXPtr(std::move(geog_out));
    }

  private:
    S2BooleanOperation::Options options;
    GeographyOperationOptions::LayerOptions layerOptions;
    s2geography::GlobalOptions geographyOptions;
  };

  Op op(s2options);
  return op.processVector(geog);
}

// [[Rcpp::export]]
List cpp_s2_interpolate_normalized(List geog, NumericVector distanceNormalized) {
  class Op: public UnaryGeographyOperator<List, SEXP> {
  public:
    NumericVector distanceNormalized;
    Op(NumericVector distanceNormalized): distanceNormalized(distanceNormalized) {}
    SEXP processFeature(XPtr<RGeography> feature, R_xlen_t i) {
      if (NumericVector::is_na(this->distanceNormalized[i])) {
        return R_NilValue;
      }

      if (s2geography::s2_is_empty(feature->Geog())) {
        return RGeography::MakeXPtr(RGeography::MakePoint());
      }

      if (s2geography::s2_is_collection(feature->Geog())) {
        throw GeographyOperatorException("`x` must be a simple geography");
      } else if (feature->Geog().dimension() != 1) {
        throw GeographyOperatorException("`x` must be a polyline");
      }

      S2Point point = s2geography::s2_interpolate_normalized(feature->Geog(), this->distanceNormalized[i]);

      if (point.Norm2() == 0) {
        return RGeography::MakeXPtr(RGeography::MakePoint());
      } else {
        return RGeography::MakeXPtr(RGeography::MakePoint(point));
      }
    }
  };

  Op op(distanceNormalized);
  return op.processVector(geog);
}

// [[Rcpp::export]]
List cpp_s2_buffer_cells(List geog, NumericVector distance, int maxCells, int minLevel) {
  class Op: public UnaryGeographyOperator<List, SEXP> {
  public:
    NumericVector distance;
    S2RegionCoverer coverer;

    Op(NumericVector distance, int maxCells, int minLevel): distance(distance) {
      this->coverer.mutable_options()->set_max_cells(maxCells);
      if (minLevel > 0) {
        this->coverer.mutable_options()->set_min_level(minLevel);
      }
    }

    SEXP processFeature(XPtr<RGeography> feature, R_xlen_t i) {
      S2ShapeIndexBufferedRegion region;
      region.Init(&feature->Index().ShapeIndex(), S1ChordAngle::Radians(this->distance[i]));

      S2CellUnion cellUnion;
      cellUnion = coverer.GetCovering(region);

      std::unique_ptr<S2Polygon> polygon = absl::make_unique<S2Polygon>();
      polygon->InitToCellUnionBorder(cellUnion);

      return RGeography::MakeXPtr(RGeography::MakePolygon(std::move(polygon)));
    }
  };

  Op op(distance, maxCells, minLevel);
  return op.processVector(geog);
}

// [[Rcpp::export]]
List cpp_s2_convex_hull(List geog) {
  class Op: public UnaryGeographyOperator<List, SEXP> {
    SEXP processFeature(XPtr<RGeography> feature, R_xlen_t i) {
      std::unique_ptr<s2geography::Geography> geog_out =
        s2geography::s2_convex_hull(feature->Geog());
      return RGeography::MakeXPtr(std::move(geog_out));
    }
  };

  Op op;
  return op.processVector(geog);
}


// [[Rcpp::export]]
List cpp_s2_convex_hull_agg(List geog, bool naRm) {
  s2geography::S2ConvexHullAggregator agg;

  SEXP item;
  for (R_xlen_t i = 0; i < geog.size(); i++) {
    item = geog[i];
    if (item == R_NilValue && !naRm) {
      return List::create(R_NilValue);
    }

    if (item != R_NilValue) {
      XPtr<RGeography> feature(item);
      agg.Add(feature->Geog());
    }
  }

  return List::create(RGeography::MakeXPtr(agg.Finalize()));
}
