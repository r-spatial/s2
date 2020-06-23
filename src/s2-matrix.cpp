
#include "s2/s2boolean_operation.h"
#include "s2/s2closest_edge_query.h"
#include "s2/s2furthest_edge_query.h"

/* for matrix2 mess: */
#include "s2/s2builder.h"
#include "s2/s2builderutil_s2polygon_layer.h"
#include "s2/s2builderutil_s2polyline_vector_layer.h"
#include "s2/s2builderutil_s2point_vector_layer.h"
#include "s2/s2builderutil_closed_set_normalizer.h"
#include "s2/s2builderutil_snap_functions.h"

#include "point-geography.h"
#include "polyline-geography.h"
#include "polygon-geography.h"
#include "geography-collection.h"
/* end matrix2 mess */
/* new: */
#include "s2/value_lexicon.h"

#include "geography-operator.h"
#include "s2-options.h"

#include <Rcpp.h>
using namespace Rcpp;



template<class VectorType, class ScalarType>
class IndexedBinaryGeographyOperator: public UnaryGeographyOperator<VectorType, ScalarType> {
public:
  MutableS2ShapeIndex geog2Index;
  std::unordered_map<int, R_xlen_t> geog2IndexSource;

  void buildIndex(List geog2) {
    SEXP item2;
    std::vector<int> shapeIds;

    for (R_xlen_t j = 0; j < geog2.size(); j++) {
      item2 = geog2[j];

      // build index and store index IDs so that shapeIds can be
      // mapped back to the geog2 index
      if (item2 == R_NilValue) {
        Rcpp::stop("Missing `y` not allowed in binary indexed operators()");
      } else {
        Rcpp::XPtr<Geography> feature2(item2);
        shapeIds = feature2->BuildShapeIndex(&(this->geog2Index));
        for (size_t k = 0; k < shapeIds.size(); k ++) {
          geog2IndexSource[shapeIds[k]] = j;
        }
      }
    }
  }
};

// -------- closest/farthest feature ----------

// [[Rcpp::export]]
IntegerVector cpp_s2_closest_feature(List geog1, List geog2) {

  class Op: public IndexedBinaryGeographyOperator<IntegerVector, int> {
  public:
    int processFeature(Rcpp::XPtr<Geography> feature, R_xlen_t i) {
      S2ClosestEdgeQuery query(&(this->geog2Index));
      S2ClosestEdgeQuery::ShapeIndexTarget target(feature->ShapeIndex());
      const auto& result = query.FindClosestEdge(&target);
      if (result.is_empty()) {
        return NA_INTEGER;
      } else {
        // convert to R index (+1)
        return this->geog2IndexSource[result.shape_id()] + 1;
      }
    }
  };

  Op op;
  op.buildIndex(geog2);
  return op.processVector(geog1);
}

// [[Rcpp::export]]
IntegerVector cpp_s2_farthest_feature(List geog1, List geog2) {

  class Op: public IndexedBinaryGeographyOperator<IntegerVector, int> {
  public:
    int processFeature(Rcpp::XPtr<Geography> feature, R_xlen_t i) {
      S2FurthestEdgeQuery query(&(this->geog2Index));
      S2FurthestEdgeQuery::ShapeIndexTarget target(feature->ShapeIndex());
      const auto& result = query.FindFurthestEdge(&target);
      if (result.is_empty()) {
        return NA_INTEGER;
      } else {
        // convert to R index (+1)
        return this->geog2IndexSource[result.shape_id()] + 1;
      }
    }
  };

  Op op;
  op.buildIndex(geog2);
  return op.processVector(geog1);
}

// ----------- binary predicate operators ------------------

class MatrixPredicateOperator {
public:
  std::vector<S2ShapeIndex*> geog2Indices;
  S2BooleanOperation::Options options;

  MatrixPredicateOperator() {}

  MatrixPredicateOperator(Rcpp::List s2options) {
    GeographyOperationOptions options(s2options);
    this->options = options.booleanOperationOptions();
  }

  void buildIndex(List geog2) {
    SEXP item2;
    this->geog2Indices = std::vector<S2ShapeIndex*>(geog2.size());

    for (R_xlen_t j = 0; j < geog2.size(); j++) {
      item2 = geog2[j];

      if (item2 == R_NilValue) {
        Rcpp::stop("Missing `y` not allowed in binary indexed operators()");
      } else {
        Rcpp::XPtr<Geography> feature2(item2);
        geog2Indices[j] = feature2->ShapeIndex();
      }
    }
  }

  List processVector(Rcpp::List geog1) {
    List output(geog1.size());
    SEXP item1;
    // using instead of IntegerVector because
    // std::vector is much faster with repeated calls to
    // push_back()
    // alternatively, could allocate one logical vector and
    // use Rcpp::which() or similar
    std::vector<int> trueIndices;

    for (R_xlen_t i = 0; i < geog1.size(); i++) {
      trueIndices.clear();

      item1 = geog1[i];
      if (item1 ==  R_NilValue) {
        output[i] = R_NilValue;
      } else {
        Rcpp::XPtr<Geography> feature1(item1);

        for (size_t j = 0; j < this->geog2Indices.size(); j++) { // here, replace the loop with a query
          bool result = this->processFeature(
            feature1->ShapeIndex(),
            this->geog2Indices[j],
            i, j
          );

          if (result) {
            // convert to R index here (+1)
            trueIndices.push_back(j + 1);
          }
        }

        IntegerVector itemOut(trueIndices.size());
        for (size_t k = 0; k < trueIndices.size(); k++) {
          itemOut[k] = trueIndices[k];
        }
        output[i] = itemOut;
      }
    }

    return output;
  }

  virtual bool processFeature(S2ShapeIndex* index1, S2ShapeIndex* index2,
                              R_xlen_t i, R_xlen_t j) = 0;
};

/*
class MyShape : public S2Shape { // see s2shape.h, line 240 ff.
  public:
    void set_id(int i) {
      geog_id_ = i;
    }
    virtual void* mutable_user_data() { return &geog_id_; }
  private:
    int geog_id_;
};

class MatrixPredicateOperator2 {
public:
  // std::vector<S2ShapeIndex*> geog2Indices;
  S2ShapeIndex* geog2Index;
  S2BooleanOperation::Options options;

  MatrixPredicateOperator2() {}

  MatrixPredicateOperator2(Rcpp::List s2options) {
    GeographyOperationOptions options(s2options);
    this->options = options.booleanOperationOptions();
  }

  MutableS2ShapeIndex collapseS2Shapes(List x) {
    SEXP item;
    MutableS2ShapeIndex m;
    for (int i; i < x.size(); i++) {
      item = x[i];
      if (item == R_NilValue) {
        Rcpp::stop("Missing `y` not allowed in binary indexed operators()");
      } else {
        Rcpp::XPtr<Geography> feature(item);
        S2ShapeIndex *si = feature->ShapeIndex();
        for (int j = 0; j < si->num_shape_ids(); j++) { // 
          MyShape *my_shape = static_cast<MyShape*>(si->shape(i));
          if (my_shape != nullptr) {
            my_shape->set_id(i);
            m.Add(my_shape);
          }
        }
      }
    }
    return m;
  }

  void buildIndex(List geog2) {
    this->geog2Index = collapseS2Shapes(geog2);
  }
    
//    for (R_xlen_t j = 0; j < geog2.size(); j++) {
//      item2 = geog2[j];
//
//      if (item2 == R_NilValue) {
//        Rcpp::stop("Missing `y` not allowed in binary indexed operators()");
//      } else {
//        Rcpp::XPtr<Geography> feature2(item2);
//        geog2Indices[j] = feature2->ShapeIndex();
//      }
//    }


  List processVector(Rcpp::List geog1) {
    List output(geog1.size());
    SEXP item1;

    for (R_xlen_t i = 0; i < geog1.size(); i++) {

      item1 = geog1[i];
      if (item1 ==  R_NilValue) {
        output[i] = R_NilValue;
      } else {
        Rcpp::XPtr<Geography> feature1(item1);

        // using instead of IntegerVector because
        // std::vector is much faster with repeated calls to
        // push_back()
        // alternatively, could allocate one logical vector and
        // use Rcpp::which() or similar
        std::vector<int> trueIndices = this->processFeature(
          feature1->ShapeIndex(),
          this->geog2Index, i);

        IntegerVector itemOut(trueIndices.size());
        for (size_t k = 0; k < trueIndices.size(); k++) { // copy:
          itemOut[k] = trueIndices[k];
        }
        output[i] = itemOut;
      }
    }

    return output;
  }

  virtual bool processFeature(S2ShapeIndex* index1, S2ShapeIndex* index2,
                              R_xlen_t i, R_xlen_t j) = 0;
}
*/

// [[Rcpp::export]]
List cpp_s2_contains_matrix(List geog1, List geog2, List s2options) {
  class Op: public MatrixPredicateOperator {
  public:
    Op(List s2options): MatrixPredicateOperator(s2options) {}
    bool processFeature(S2ShapeIndex* index1, S2ShapeIndex* index2, R_xlen_t i, R_xlen_t j) {
      return S2BooleanOperation::Contains(*index1, *index2, this->options);
    };
  };

  Op op(s2options);
  op.buildIndex(geog2);
  return op.processVector(geog1);
}

// [[Rcpp::export]]
List cpp_s2_within_matrix(List geog1, List geog2, List s2options) {
  class Op: public MatrixPredicateOperator {
  public:
    Op(List s2options): MatrixPredicateOperator(s2options) {}
    bool processFeature(S2ShapeIndex* index1, S2ShapeIndex* index2, R_xlen_t i, R_xlen_t j) {
      return S2BooleanOperation::Contains(*index2, *index1, this->options);
    };
  };

  Op op(s2options);
  op.buildIndex(geog2);
  return op.processVector(geog1);
}

// [[Rcpp::export]]
List cpp_s2_intersects_matrix(List geog1, List geog2, List s2options) {
  class Op: public MatrixPredicateOperator {
  public:
    Op(List s2options): MatrixPredicateOperator(s2options) {}
    bool processFeature(S2ShapeIndex* index1, S2ShapeIndex* index2, R_xlen_t i, R_xlen_t j) {
      return S2BooleanOperation::Intersects(*index1, *index2, this->options);
    };
  };

  Op op(s2options);
  op.buildIndex(geog2);
  return op.processVector(geog1);
}

// [[Rcpp::export]]
List cpp_s2_disjoint_matrix(List geog1, List geog2, List s2options) {
  class Op: public MatrixPredicateOperator {
  public:
    Op(List s2options): MatrixPredicateOperator(s2options) {}
    bool processFeature(S2ShapeIndex* index1, S2ShapeIndex* index2, R_xlen_t i, R_xlen_t j) {
      return !S2BooleanOperation::Intersects(*index1, *index2, this->options);
    };
  };

  Op op(s2options);
  op.buildIndex(geog2);
  return op.processVector(geog1);
}

// [[Rcpp::export]]
List cpp_s2_equals_matrix(List geog1, List geog2, List s2options) {
  class Op: public MatrixPredicateOperator {
  public:
    Op(List s2options): MatrixPredicateOperator(s2options) {}
    bool processFeature(S2ShapeIndex* index1, S2ShapeIndex* index2, R_xlen_t i, R_xlen_t j) {
      return S2BooleanOperation::Equals(*index1, *index2, this->options);
    };
  };

  Op op(s2options);
  op.buildIndex(geog2);
  return op.processVector(geog1);
}

// [[Rcpp::export]]
List cpp_s2_dwithin_matrix(List geog1, List geog2, double distance) {
  class Op: public MatrixPredicateOperator {
  public:
    double distance;
    Op(double distance): distance(distance) {}
    bool processFeature(S2ShapeIndex* index1, S2ShapeIndex* index2, R_xlen_t i, R_xlen_t j) {
      S2ClosestEdgeQuery query(index2);
      S2ClosestEdgeQuery::ShapeIndexTarget target(index1);
      return query.IsDistanceLessOrEqual(&target, S1ChordAngle::Radians(this->distance));
    };
  };

  Op op(distance);
  op.buildIndex(geog2);
  return op.processVector(geog1);
}

// ----------- distance matrix operators -------------------

template<class MatrixType, class ScalarType>
class MatrixGeographyOperator {
public:
  MatrixType processVector(Rcpp::List geog1, Rcpp::List geog2) {

    MatrixType output(geog1.size(), geog2.size());

    SEXP item1;
    SEXP item2;

    for (R_xlen_t i = 0; i < geog1.size(); i++) {
      item1 = geog1[i];
      if (item1 ==  R_NilValue) {
        for (R_xlen_t j = 0; j < geog2.size(); j++) {
          output(i, j) = MatrixType::get_na();
        }
      } else {
        Rcpp::XPtr<Geography> feature1(item1);

        for (R_xlen_t j = 0; j < geog2.size(); j++) {
          item2 = geog2[j];

          if (item2 == R_NilValue) {
            output(i, j) = MatrixType::get_na();
          } else {
            Rcpp::XPtr<Geography> feature2(item2);
            output(i, j) = this->processFeature(feature1, feature2, i, j);
          }
        }
      }
    }

    return output;
  }

  virtual ScalarType processFeature(Rcpp::XPtr<Geography> feature1,
                                    Rcpp::XPtr<Geography> feature2,
                                    R_xlen_t i, R_xlen_t j) = 0;
};

// [[Rcpp::export]]
NumericMatrix cpp_s2_distance_matrix(List geog1, List geog2) {
  class Op: public MatrixGeographyOperator<NumericMatrix, double> {

    double processFeature(XPtr<Geography> feature1,
                          XPtr<Geography> feature2,
                          R_xlen_t i, R_xlen_t j) {
      S2ClosestEdgeQuery query(feature1->ShapeIndex());
      S2ClosestEdgeQuery::ShapeIndexTarget target(feature2->ShapeIndex());
      const auto& result = query.FindClosestEdge(&target);

      S1ChordAngle angle = result.distance();
      double distance = angle.ToAngle().radians();

      if (distance == R_PosInf) {
        return NA_REAL;
      } else {
        return distance;
      }
    }
  };

  Op op;
  return op.processVector(geog1, geog2);
}

// [[Rcpp::export]]
NumericMatrix cpp_s2_max_distance_matrix(List geog1, List geog2) {
  class Op: public MatrixGeographyOperator<NumericMatrix, double> {

    double processFeature(XPtr<Geography> feature1,
                          XPtr<Geography> feature2,
                          R_xlen_t i, R_xlen_t j) {
      S2FurthestEdgeQuery query(feature1->ShapeIndex());
      S2FurthestEdgeQuery::ShapeIndexTarget target(feature2->ShapeIndex());
      const auto& result = query.FindFurthestEdge(&target);

      S1ChordAngle angle = result.distance();
      double distance = angle.ToAngle().radians();

      // returns -1 if one of the indexes is empty
      // NA is more consistent with the BigQuery
      // function, and makes way more sense
      if (distance < 0) {
        return NA_REAL;
      } else {
        return distance;
      }
    }
  };

  Op op;
  return op.processVector(geog1, geog2);
}


// matrix2 mess:
template<class VectorType, class ScalarType>
class BinaryGeographyOperator2 {
public:
// xxx
  VectorType processVector(Rcpp::List geog, int) {
    VectorType output(geog.size());

    Rcout << "processing one element...\n";
    SEXP item;
    for (R_xlen_t i = 0; i < geog.size(); i++) {
      item = geog[i];
      if (item == R_NilValue) {
        // output[i] = VectorType::get_na();
        output[i] = R_NilValue;
      } else {
        Rcpp::XPtr<Geography> feature(item);
        output[i] = this->processFeature(feature, i);
      }
    }

    return output;
  }

  virtual IntegerVector processFeature(Rcpp::XPtr<Geography> feature1, R_xlen_t i) = 0;
};

template<class VectorType, class ScalarType>
class IndexedBinaryGeographyOperator2: public BinaryGeographyOperator2<VectorType, ScalarType> {
public:
  MutableS2ShapeIndex geog2Index;
  std::unordered_map<int, R_xlen_t> geog2IndexSource;

  void buildIndex(List geog2) {
    SEXP item2;
    std::vector<int> shapeIds;

    for (R_xlen_t j = 0; j < geog2.size(); j++) {
      item2 = geog2[j];

      // build index and store index IDs so that shapeIds can be
      // mapped back to the geog2 index
      if (item2 == R_NilValue) {
        Rcpp::stop("Missing `y` not allowed in binary indexed operators()");
      } else {
        Rcpp::XPtr<Geography> feature2(item2);
        shapeIds = feature2->BuildShapeIndex(&(this->geog2Index));
        for (size_t k = 0; k < shapeIds.size(); k ++) {
          // Rcout << "indexing[" << shapeIds[k] << "]: " << j << std::endl;
          geog2IndexSource[shapeIds[k]] = j;
        }
      }
    }
  }
};

// List cpp_s2_intersects_matrix2(List geog1, List geog2, List s2options) {

// [[Rcpp::export]]
List cpp_s2_intersects_matrix2(List geog1, List geog2) {

  class Op: public IndexedBinaryGeographyOperator2<List, int> {
  public:
    IntegerVector processFeature(Rcpp::XPtr<Geography> feature, R_xlen_t i_) {
      // do the intersection of each of geog1 with geog2
      // BooleanOperationOp op(S2BooleanOperation::OpType::INTERSECTION, s2options);

      // handle s2options!
      S2BooleanOperation::Options options;
      ValueLexicon<S2BooleanOperation::SourceId> *source = options.source_id_lexicon();
      //options.set_source_id_lexicon(ValueLexicon<SourceId>* source_id_lexicon);
      // options.set_source_id_lexicon(source);
      Rcout << "source_id_lexicon: " << source << std::endl;

      // create the data structures that will contain the output
      std::vector<S2Point> points;
      std::vector<std::unique_ptr<S2Polyline>> polylines;
      std::unique_ptr<S2Polygon> polygon = absl::make_unique<S2Polygon>();
  
      // s2builderutil::LayerVector layers(3);
      std::vector<std::unique_ptr<S2Builder::Layer>> layers(3);

      // fill these 3:
      std::vector<int32> points_LabelSetIds;
      IdSetLexicon points_IdSetLexicon;
      layers[0] = absl::make_unique<s2builderutil::S2PointVectorLayer>(&points, 
        &points_LabelSetIds, &points_IdSetLexicon, s2builderutil::S2PointVectorLayer::Options());

      std::vector<std::vector<int32>> polylines_LabelSetIds; // set of sets
      IdSetLexicon polylines_IdSetLexicon;
      layers[1] = absl::make_unique<s2builderutil::S2PolylineVectorLayer>(&polylines,
        &polylines_LabelSetIds, &polylines_IdSetLexicon, s2builderutil::S2PolylineVectorLayer::Options());

      std::vector<std::vector<int32>> polygon_LabelSetIds; // set of sets
      IdSetLexicon polygon_IdSetLexicon;
      layers[2] = absl::make_unique<s2builderutil::S2PolygonLayer>(polygon.get(),
        &polygon_LabelSetIds, &polygon_IdSetLexicon, s2builderutil::S2PolygonLayer::Options());
  
      // do the boolean operation
      S2BooleanOperation booleanOp(S2BooleanOperation::OpType::INTERSECTION, 
      // normalizing the closed set here is required for line intersections
      // to work as expected
        // s2builderutil::NormalizeClosedSet(std::move(layers)),
        std::move(layers),
        options
      );

      // check for errors
      S2Error error;
      S2ShapeIndex *fsi = feature->ShapeIndex();
      if (!booleanOp.Build(*fsi,
          // &(this->geog2Index), 
          *(static_cast<const S2ShapeIndex*>(&(this->geog2Index))),
          &error)) {
        stop(error.text());
      }
      /* now get shape_id's from the three resulting layers: */
	  // TBD

      std::vector<int> ids;
      IntegerVector ret(ids.size());
      return ret;
    }
  };

  Op op;
  op.buildIndex(geog2);
  return op.processVector(geog1, 0);
}
