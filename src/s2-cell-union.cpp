
#include "s2/s2cell_id.h"
#include "s2/s2cell.h"
#include "s2/s2latlng.h"
#include "s2/s2cell_union.h"

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
  uint64_t* cellIds = (uint64_t*) &(cellIdNumeric[0]);
  std::vector<uint64_t> cellIdsVector(cellIds, cellIds + cellIdNumeric.size());
  return S2CellUnion(cellIdsVector);
}

NumericVector cell_id_vector_from_cell_union(const S2CellUnion& cellUnion) {
  NumericVector cellIdNumeric(cellUnion.size());
  for (R_xlen_t i = 0; i < cellIdNumeric.size(); i++) {
    cellIdNumeric = reinterpret_double(cellUnion.cell_id(i).id());
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
