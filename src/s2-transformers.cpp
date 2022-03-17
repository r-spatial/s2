
#include "s2/s2shape_index_buffered_region.h"
#include "s2/s2region_coverer.h"

#include "s2-options.h"
#include "geography-operator.h"
#include "geography-shim.h"

#include <Rcpp.h>
using namespace Rcpp;


class BooleanOperationOp: public BinaryGeographyOperator<List, SEXP> {
public:
  BooleanOperationOp(S2BooleanOperation::OpType opType, List s2options):
    opType(opType) {
      GeographyOperationOptions options(s2options);
      this->geography_options = options.geographyOptions();
    }

  SEXP processFeature(XPtr<Geography> feature1, XPtr<Geography> feature2, R_xlen_t i) {
    auto geog1 = feature1->NewGeography();
    auto geog2 = feature2->NewGeography();
    s2geography::S2GeographyShapeIndex index1(*geog1);
    s2geography::S2GeographyShapeIndex index2(*geog2);

    std::unique_ptr<s2geography::S2Geography> geog_out = s2geography::s2_boolean_operation(
      index1, index2,
      this->opType,
      this->geography_options);

    std::unique_ptr<Geography> geography = MakeOldGeography(*geog_out);
    return Rcpp::XPtr<Geography>(geography.release());
  }

private:
  S2BooleanOperation::OpType opType;
  s2geography::S2GeographyOptions geography_options;
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
  std::vector<std::unique_ptr<s2geography::S2Geography>> geographies;

  SEXP item;
  for (R_xlen_t i = 0; i < geog.size(); i++) {
    item = geog[i];
    if (item == R_NilValue && !naRm) {
      return List::create(R_NilValue);
    }

    if (item != R_NilValue) {
      Rcpp::XPtr<Geography> feature(item);
      auto geog = feature->NewGeography();
      agg.Add(*geog);
      geographies.push_back(std::move(geog));
    }
  }

  std::unique_ptr<s2geography::S2Geography> geog_out = agg.Finalize();

  auto geography = MakeOldGeography(*geog_out);
  return List::create(Rcpp::XPtr<Geography>(geography.release()));
}

// [[Rcpp::export]]
List cpp_s2_union_agg(List geog, List s2options, bool naRm) {
  GeographyOperationOptions options(s2options);
  s2geography::S2UnionAggregator agg(options.geographyOptions());
  std::vector<std::unique_ptr<s2geography::S2Geography>> geographies;

  SEXP item;
  for (R_xlen_t i = 0; i < geog.size(); i++) {
    item = geog[i];
    if (item == R_NilValue && !naRm) {
      return List::create(R_NilValue);
    }

    if (item != R_NilValue) {
      Rcpp::XPtr<Geography> feature(item);
      auto geog = feature->NewGeography();
      agg.Add(*geog);
      geographies.push_back(std::move(geog));
    }
  }

  std::unique_ptr<s2geography::S2Geography> geog_out = agg.Finalize();

  auto geography = MakeOldGeography(*geog_out);
  return List::create(Rcpp::XPtr<Geography>(geography.release()));
}

// [[Rcpp::export]]
List cpp_s2_centroid_agg(List geog, bool naRm) {
  s2geography::S2CentroidAggregator agg;

  SEXP item;
  for (R_xlen_t i = 0; i < geog.size(); i++) {
    item = geog[i];
    if (item == R_NilValue && !naRm) {
      return List::create(R_NilValue);
    }

    if (item != R_NilValue) {
      Rcpp::XPtr<Geography> feature(item);
      agg.Add(*feature->NewGeography());
    }
  }

  S2Point centroid = agg.Finalize();

  List output(1);
  if (centroid.Norm2() == 0) {
    output[0] = Rcpp::XPtr<Geography>(new PointGeography());
  } else {
    output[0] = Rcpp::XPtr<Geography>(new PointGeography(centroid));
  }

  return output;
}

// [[Rcpp::export]]
List cpp_s2_rebuild_agg(List geog, List s2options, bool naRm) {
  GeographyOperationOptions options(s2options);

  s2geography::S2RebuildAggregator agg(options.geographyOptions());
  std::vector<std::unique_ptr<s2geography::S2Geography>> geographies;

  SEXP item;
  for (R_xlen_t i = 0; i < geog.size(); i++) {
    item = geog[i];
    if (item == R_NilValue && !naRm) {
      return List::create(R_NilValue);
    }

    if (item != R_NilValue) {
      Rcpp::XPtr<Geography> feature(item);
      auto geog = feature->NewGeography();
      agg.Add(*geog);
      geographies.push_back(std::move(geog));
    }
  }

  auto geog_out = agg.Finalize();

  auto geography = MakeOldGeography(*geog_out);
  return List::create(Rcpp::XPtr<Geography>(geography.release()));
}

// [[Rcpp::export]]
List cpp_s2_closest_point(List geog1, List geog2) {
  class Op: public BinaryGeographyOperator<List, SEXP> {

    SEXP processFeature(XPtr<Geography> feature1, XPtr<Geography> feature2, R_xlen_t i) {
      auto geog1 = feature1->NewGeography();
      auto geog2 = feature2->NewGeography();
      s2geography::S2GeographyShapeIndex index1(*geog1);
      s2geography::S2GeographyShapeIndex index2(*geog2);

      S2Point pt = s2geography::s2_closest_point(index1, index2);
      if (pt.Norm2() == 0) {
        return XPtr<Geography>(new PointGeography());
      } else {
        return XPtr<Geography>(new PointGeography(pt));
      }
    }
  };

  Op op;
  return op.processVector(geog1, geog2);
}

// [[Rcpp::export]]
List cpp_s2_minimum_clearance_line_between(List geog1, List geog2) {
  class Op: public BinaryGeographyOperator<List, SEXP> {

    SEXP processFeature(XPtr<Geography> feature1, XPtr<Geography> feature2, R_xlen_t i) {
      auto geog1 = feature1->NewGeography();
      auto geog2 = feature2->NewGeography();
      s2geography::S2GeographyShapeIndex index1(*geog1);
      s2geography::S2GeographyShapeIndex index2(*geog2);

      std::pair<S2Point, S2Point> pts = s2geography::s2_minimum_clearance_line_between(
        index1,
        index2
      );

      if (pts.first.Norm2() == 0) {
        return XPtr<Geography>(new PolylineGeography());
      }

      std::vector<S2Point> vertices(2);
      vertices[0] = pts.first;
      vertices[1] = pts.second;

      if (pts.first == pts.second) {
        return XPtr<Geography>(new PointGeography(vertices));
      } else {
        std::vector<S2Point> vertices(2);
        vertices[0] = pts.first;
        vertices[1] = pts.second;
        std::unique_ptr<S2Polyline> polyline = absl::make_unique<S2Polyline>();
        polyline->Init(vertices);
        std::vector<std::unique_ptr<S2Polyline>> polylines(1);
        polylines[0] = std::move(polyline);
        return XPtr<Geography>(new PolylineGeography(std::move(polylines)));
      }
    }
  };

  Op op;
  return op.processVector(geog1, geog2);
}

// [[Rcpp::export]]
List cpp_s2_centroid(List geog) {
  class Op: public UnaryGeographyOperator<List, SEXP> {
    SEXP processFeature(XPtr<Geography> feature, R_xlen_t i) {
      auto geog = feature->NewGeography();
      S2Point centroid = s2geography::s2_centroid(*geog);
      if (centroid.Norm2() == 0) {
        return XPtr<Geography>(new PointGeography());
      } else {
        return XPtr<Geography>(new PointGeography(centroid.Normalize()));
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

    SEXP processFeature(XPtr<Geography> feature, R_xlen_t i) {
      auto geog = feature->NewGeography();
      S2Point result = s2geography::s2_point_on_surface(*geog, coverer);
      if (result.Norm2() == 0) {
        return XPtr<Geography>(new PointGeography());
      } else {
        return XPtr<Geography>(new PointGeography(result));
      }
    }
  };

  Op op;
  return op.processVector(geog);
}

// [[Rcpp::export]]
List cpp_s2_boundary(List geog) {
  class Op: public UnaryGeographyOperator<List, SEXP> {
    SEXP processFeature(XPtr<Geography> feature, R_xlen_t i) {
      auto geog = feature->NewGeography();
      std::unique_ptr<s2geography::S2Geography> result = s2geography::s2_boundary(*geog);
      std::unique_ptr<Geography> ptr = MakeOldGeography(*result);
      return XPtr<Geography>(ptr.release());
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

    SEXP processFeature(XPtr<Geography> feature, R_xlen_t i) {
      auto geog = feature->NewGeography();
      s2geography::S2GeographyShapeIndex index(*geog);
      std::unique_ptr<s2geography::S2Geography> ptr = s2geography::s2_rebuild(
        index,
        this->options
      );

      auto geography = MakeOldGeography(*ptr);
      return XPtr<Geography>(geography.release());
    }

  private:
    s2geography::S2GeographyOptions options;
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

    SEXP processFeature(XPtr<Geography> feature, R_xlen_t i) {
      auto geog = feature->NewGeography();
      s2geography::S2GeographyShapeIndex index(*geog);
      std::unique_ptr<s2geography::S2Geography> geog_out =
        s2geography::s2_unary_union(index, this->geographyOptions);
      auto geography = MakeOldGeography(*geog_out);
      return XPtr<Geography>(geography.release());
    }

  private:
    S2BooleanOperation::Options options;
    GeographyOperationOptions::LayerOptions layerOptions;
    s2geography::S2GeographyOptions geographyOptions;
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
    SEXP processFeature(XPtr<Geography> feature, R_xlen_t i) {
      if (NumericVector::is_na(this->distanceNormalized[i])) {
        return R_NilValue;
      }

      auto geog = feature->NewGeography();

      if (s2geography::s2_is_empty(*geog)) {
        return XPtr<PointGeography>(new PointGeography());
      }

      if (s2geography::s2_is_collection(*geog)) {
        throw GeographyOperatorException("`x` must be a simple geography");
      } else if (geog->dimension() != 1) {
        throw GeographyOperatorException("`x` must be a polyline");
      }

      S2Point point = s2geography::s2_interpolate_normalized(*geog, this->distanceNormalized[i]);

      if (point.Norm2() == 0) {
        return XPtr<PointGeography>(new PointGeography());
      } else {
        return XPtr<PointGeography>(new PointGeography(point));
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

    SEXP processFeature(XPtr<Geography> feature, R_xlen_t i) {
      auto geog = feature->NewGeography();
      s2geography::S2GeographyShapeIndex index(*geog);

      S2ShapeIndexBufferedRegion region;
      region.Init(&index.ShapeIndex(), S1ChordAngle::Radians(this->distance[i]));

      S2CellUnion cellUnion;
      cellUnion = coverer.GetCovering(region);

      std::unique_ptr<S2Polygon> polygon = absl::make_unique<S2Polygon>();
      polygon->InitToCellUnionBorder(cellUnion);

      return XPtr<PolygonGeography>(new PolygonGeography(std::move(polygon)));
    }
  };

  Op op(distance, maxCells, minLevel);
  return op.processVector(geog);
}

// [[Rcpp::export]]
List cpp_s2_convex_hull(List geog) {
  class Op: public UnaryGeographyOperator<List, SEXP> {
    SEXP processFeature(XPtr<Geography> feature, R_xlen_t i) {
      auto geog = feature->NewGeography();
      std::unique_ptr<s2geography::S2Geography> geog_out =
        s2geography::s2_convex_hull(*geog);
      auto geography = MakeOldGeography(*geog_out);
      return XPtr<Geography>(geography.release());
    }
  };

  Op op;
  return op.processVector(geog);
}


// [[Rcpp::export]]
List cpp_s2_convex_hull_agg(List geog, bool naRm) {
  s2geography::S2ConvexHullAggregator agg;
  std::vector<std::unique_ptr<s2geography::S2Geography>> keep_alive_;

  SEXP item;
  for (R_xlen_t i = 0; i < geog.size(); i++) {
    item = geog[i];
    if (item == R_NilValue && !naRm) {
      return List::create(R_NilValue);
    }

    if (item != R_NilValue) {
      XPtr<Geography> feature(item);
      keep_alive_.push_back(feature->NewGeography());
      agg.Add(*keep_alive_.back());
    }
  }

  auto geography = MakeOldGeography(*agg.Finalize());
  XPtr<Geography> outG(geography.release());
  return List::create(outG);
}
