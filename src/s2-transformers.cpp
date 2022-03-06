
#include "s2/s2boolean_operation.h"
#include "s2/s2closest_edge_query.h"
#include "s2/s2polygon.h"
#include "s2/s2polyline.h"
#include "s2/s2point.h"
#include "s2/s2error.h"
#include "s2/s2boolean_operation.h"
#include "s2/s2builder.h"
#include "s2/s2builderutil_s2polygon_layer.h"
#include "s2/s2builderutil_s2polyline_vector_layer.h"
#include "s2/s2builderutil_s2point_vector_layer.h"
#include "s2/s2builderutil_closed_set_normalizer.h"
#include "s2/s2builderutil_snap_functions.h"
#include "s2/s2shape_index_buffered_region.h"
#include "s2/s2region_coverer.h"
#include "s2/s2convex_hull_query.h"

#include "s2-options.h"
#include "geography-operator.h"
#include "point-geography.h"
#include "polyline-geography.h"
#include "polygon-geography.h"
#include "geography-collection.h"
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
      this->options = options.booleanOperationOptions();
      this->layerOptions = options.layerOptions();
      this->geographyOptions = options.geographyOptions();
    }

    SEXP processFeature(XPtr<Geography> feature, R_xlen_t i) {
      // complex union only needed when a polygon is involved
      bool simpleUnionOK = feature->IsEmpty() ||
        (feature->Dimension() < 2);

      // valid polygons that are not part of a collection can also use a
      // simple union (common)
      if (feature->GeographyType() == Geography::Type::GEOGRAPHY_POLYGON) {
        S2Error validationError;
        if(!(feature->Polygon()->FindValidationError(&validationError))) {
          simpleUnionOK = true;
        }
      }

      if (simpleUnionOK) {
        auto geog = feature->NewGeography();
        s2geography::S2GeographyShapeIndex index(*geog);
        s2geography::S2GeographyShapeIndex emptyIndex;

        std::unique_ptr<s2geography::S2Geography> geog_out = s2geography::s2_boolean_operation(
          index,
          emptyIndex,
          S2BooleanOperation::OpType::UNION,
          this->geographyOptions
        );

        auto geography = MakeOldGeography(*geog_out);

        return XPtr<Geography>(geography.release());
      } else if (feature->GeographyType() == Geography::Type::GEOGRAPHY_POLYGON) {
        // If we've made it here we have an invalid polygon on our hands. A geography with
        // invalid loops won't work with the S2BooleanOperation we will use to accumulate
        // (i.e., union) valid polygons, so we need to rebuild each loop as its own polygon,
        // splitting crossed edges along the way.
        const S2Polygon* originalPoly = feature->Polygon();

        // Not exposing these options as an argument (except snap function)
        // because a particular combiation of them is required for this to work
        S2Builder::Options builderOptions;
        builderOptions.set_split_crossing_edges(true);
        builderOptions.set_snap_function(this->options.snap_function());
        s2builderutil::S2PolygonLayer::Options layerOptions;
        layerOptions.set_edge_type(S2Builder::EdgeType::UNDIRECTED);
        layerOptions.set_validate(false);

        // Rebuild all loops as polygons using the S2Builder()
        std::vector<std::unique_ptr<S2Polygon>> loops;
        for (int i = 0; i < originalPoly->num_loops(); i++) {
          std::unique_ptr<S2Polygon> loop = absl::make_unique<S2Polygon>();
          S2Builder builder(builderOptions);
          builder.StartLayer(absl::make_unique<s2builderutil::S2PolygonLayer>(loop.get()));
          builder.AddShape(S2Loop::Shape(originalPoly->loop(i)));
          S2Error error;
          if (!builder.Build(&error)) {
            throw GeographyOperatorException(error.text());
          }

          // Check if the builder created a polygon whose boundary contained more than
          // half the earth (and invert it if so)
          if (loop->GetArea() > (2 * M_PI)) {
            loop->Invert();
          }

          loops.push_back(std::move(loop));
        }

        // Accumulate the union of outer loops (but difference of inner loops)
        std::unique_ptr<S2Polygon> accumulatedPolygon = absl::make_unique<S2Polygon>();
        for (int i = 0; i < originalPoly->num_loops(); i++) {
          std::unique_ptr<S2Polygon> polygonResult = absl::make_unique<S2Polygon>();

          // Use original nesting to suggest if this loop should be unioned or diffed.
          // For valid polygons loops are arranged such that the biggest loop is on the outside
          // followed by holes such that the below strategy should work (since we are
          // just iterating along the original loop structure)
          if ((originalPoly->loop(i)->depth() % 2) == 0) {
            polygonResult->InitToUnion(accumulatedPolygon.get(), loops[i].get());
          } else {
            polygonResult->InitToDifference(accumulatedPolygon.get(), loops[i].get());
          }

          accumulatedPolygon.swap(polygonResult);
        }

        return XPtr<Geography>(new PolygonGeography(std::move(accumulatedPolygon)));
      } else {
        // This is a less common case (mixed dimension output that includes a polygon).
        // In the absence of a clean solution, saving this battle for another day.
        throw GeographyOperatorException("Unary union for collections is not implemented");
      }
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

      if (feature->IsCollection()) {
        throw GeographyOperatorException("`x` must be a simple geography");
      }

      if (feature->IsEmpty()) {
        return R_NilValue;
      }

      if (feature->GeographyType() == Geography::Type::GEOGRAPHY_POLYLINE) {
        S2Point point = feature->Polyline()->at(0)->Interpolate(this->distanceNormalized[i]);
        return XPtr<PointGeography>(new PointGeography(point));
      } else {
        throw GeographyOperatorException("`x` must be a polyline geography");
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
      S2ShapeIndexBufferedRegion region;
      region.Init(feature->ShapeIndex(), S1ChordAngle::Radians(this->distance[i]));

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


class ConvexHullGeographyQuery: public S2ConvexHullQuery {
public:

  void AddGeography(Geography* feature) {
    if (feature->GeographyType() == Geography::Type::GEOGRAPHY_POLYGON) {
      const S2Polygon* pol = feature->Polygon();
      this->AddPolygon(*pol);
    } else if (feature->GeographyType() == Geography::Type::GEOGRAPHY_POINT) {
      const std::vector<S2Point>* pts = feature->Point();
      for(const auto& pt: *pts) {
        this->AddPoint(pt);
      }
    } else if (feature->GeographyType() == Geography::Type::GEOGRAPHY_POLYLINE) {
      const std::vector<std::unique_ptr<S2Polyline>>* lins = feature->Polyline();
      for(const auto& lin: *lins) {
        this->AddPolyline(*lin);
      }
    } else if (feature->GeographyType() == Geography::Type::GEOGRAPHY_COLLECTION) {
      const std::vector<std::unique_ptr<Geography>>* features = feature->CollectionFeatures();
      for (const auto& feat: *features) {
        this->AddGeography(feat.get());
      }
    }
  }

  std::unique_ptr<S2Polygon> GetConvexHullPolygon() {
    return absl::make_unique<S2Polygon>(this->GetConvexHull());
  }
};

// [[Rcpp::export]]
List cpp_s2_convex_hull(List geog) {
  class Op: public UnaryGeographyOperator<List, SEXP> {
    SEXP processFeature(XPtr<Geography> feature, R_xlen_t i) {
      ConvexHullGeographyQuery convexHullQuery;
      convexHullQuery.AddGeography(feature);

      std::unique_ptr<S2Polygon> outP = convexHullQuery.GetConvexHullPolygon();
      return XPtr<Geography>(new PolygonGeography(std::move(outP)));
    }
  };

  Op op;
  return op.processVector(geog);
}


// [[Rcpp::export]]
List cpp_s2_convex_hull_agg(List geog, bool naRm) {
  // create the convex hull query
  ConvexHullGeographyQuery convexHullQuery;
  SEXP item;
  for (R_xlen_t i = 0; i < geog.size(); i++) {
    item = geog[i];
    if (item == R_NilValue && !naRm) {
      return List::create(R_NilValue);
    }

    if (item != R_NilValue) {
      XPtr<Geography> feature(item);
      convexHullQuery.AddGeography(feature);
    }
  }

  std::unique_ptr<S2Polygon> outP = convexHullQuery.GetConvexHullPolygon();
  XPtr<Geography> outG(new PolygonGeography(std::move(outP)));
  return List::create(outG);
}
