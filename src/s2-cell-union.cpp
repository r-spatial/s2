
#include "s2/s2cell_id.h"
#include "s2/s2cell.h"
#include "s2/s2latlng.h"
#include "s2/s2cell_union.h"
#include "s2/s2region_coverer.h"
#include "s2/s2shape_index_buffered_region.h"

#include "geography-operator.h"
#include "point-geography.h"
#include "polyline-geography.h"
#include "polygon-geography.h"

#include <Rcpp.h>
using namespace Rcpp;


static inline double reinterpret_double(uint64_t id) {
  double doppelganger;
  memcpy(&doppelganger, &id, sizeof(double));
  return doppelganger;
}

S2CellUnion cell_union_from_cell_id_vector(NumericVector cellIdNumeric) {
  uint64* cellIds = (uint64*) &(cellIdNumeric[0]);
  std::vector<uint64> cellIdsVector(cellIds, cellIds + cellIdNumeric.size());
  return S2CellUnion(cellIdsVector);
}

NumericVector cell_id_vector_from_cell_union(const S2CellUnion& cellUnion) {
  NumericVector cellIdNumeric(cellUnion.size());
  for (R_xlen_t i = 0; i < cellIdNumeric.size(); i++) {
    cellIdNumeric[i] = reinterpret_double(cellUnion.cell_id(i).id());
  }

  cellIdNumeric.attr("class") = CharacterVector::create("s2_cell", "wk_vctr");
  return cellIdNumeric;
}


class S2CellUnionOperatorException: public std::runtime_error {
public:
  S2CellUnionOperatorException(std::string msg): std::runtime_error(msg.c_str()) {}
};


template<class VectorType, class ScalarType>
class UnaryS2CellUnionOperator {
public:
  VectorType processVector(Rcpp::List cellUnionVector) {
    VectorType output(cellUnionVector.size());

    SEXP item;
    for (R_xlen_t i = 0; i < cellUnionVector.size(); i++) {
      if ((i % 1000) == 0) {
        Rcpp::checkUserInterrupt();
      }

      item = cellUnionVector[i];
      if (item == R_NilValue) {
        output[i] = VectorType::get_na();
      } else {
        Rcpp::NumericVector cellIdNumeric(item);
        uint64_t* cellIds = (uint64_t*) &(cellIdNumeric[0]);
        std::vector<uint64_t> cellIdsVector(cellIds, cellIds + cellIdNumeric.size());
        S2CellUnion cellUnion(cellIdsVector);

        output[i] = this->processCell(cellUnion, i);
      }
    }

    return output;
  }

  virtual ScalarType processCell(S2CellUnion& cellUnion, R_xlen_t i) = 0;
};


// [[Rcpp::export]]
List cpp_s2_cell_union_normalize(List cellUnionVector) {
  class Op: public UnaryS2CellUnionOperator<List, SEXP> {
    SEXP processCell(S2CellUnion& cellUnion, R_xlen_t i) {
      cellUnion.Normalize();
      return cell_id_vector_from_cell_union(cellUnion);
    }
  };

  Op op;
  List out = op.processVector(cellUnionVector);
  out.attr("class") = CharacterVector::create("s2_cell_union", "wk_vctr");
  return out;
}


// [[Rcpp::export]]
List cpp_s2_geography_from_cell_union(List cellUnionVector) {
  class Op: public UnaryS2CellUnionOperator<List, SEXP> {
    SEXP processCell(S2CellUnion& cellUnion, R_xlen_t i) {
      std::unique_ptr<S2Polygon> polygon = absl::make_unique<S2Polygon>();
      polygon->InitToCellUnionBorder(cellUnion);
      return XPtr<PolygonGeography>(new PolygonGeography(std::move(polygon)));
    }
  };

  Op op;
  return op.processVector(cellUnionVector);
}


// [[Rcpp::export]]
List cpp_s2_covering_cell_ids(List geog, int min_level, int max_level,
                              int max_cells, NumericVector buffer, bool interior) {
  class Op: public UnaryGeographyOperator<List, SEXP> {
  public:
    NumericVector distance;
    S2RegionCoverer& coverer;
    bool interior;

    Op(NumericVector distance, S2RegionCoverer& coverer, bool interior):
      distance(distance), coverer(coverer), interior(interior) {}

    SEXP processFeature(XPtr<Geography> feature, R_xlen_t i) {
      S2ShapeIndexBufferedRegion region;
      region.Init(feature->ShapeIndex(), S1ChordAngle::Radians(this->distance[i]));

      S2CellUnion cellUnion;
      if (interior) {
        cellUnion = coverer.GetInteriorCovering(region);
      } else {
        cellUnion = coverer.GetCovering(region);
      }

      return cell_id_vector_from_cell_union(cellUnion);
    }
  };

  S2RegionCoverer coverer;
  coverer.mutable_options()->set_min_level(min_level);
  coverer.mutable_options()->set_max_level(max_level);
  coverer.mutable_options()->set_max_cells(max_cells);

  Op op(buffer, coverer, interior);
  List out = op.processVector(geog);
  out.attr("class") = CharacterVector::create("s2_cell_union", "wk_vctr");
  return out;
}
