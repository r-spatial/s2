
#include "s2/s2boolean_operation.h"
#include "s2/s2closest_edge_query.h"

#include "geography-operator.h"
#include "s2-options.h"

#include <Rcpp.h>
using namespace Rcpp;

// [[Rcpp::export]]
IntegerVector cpp_s2_closest_feature(List geog1, List geog2) {

  class Op: public UnaryGeographyOperator<IntegerVector, int> {
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
          Rcpp::stop("Missing `y` not allowed in s2_closest_feature()");
        } else {
          Rcpp::XPtr<Geography> feature2(item2);
          shapeIds = feature2->BuildShapeIndex(&(this->geog2Index));
          for (size_t k = 0; k < shapeIds.size(); k ++) {
            geog2IndexSource[shapeIds[k]] = j;
          }
        }
      }
    }

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
