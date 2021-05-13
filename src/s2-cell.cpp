
#include <cstdint>
#include <vector>
#include <sstream>
#include <algorithm>
#include <set>

#include "s2/s2cell_id.h"
#include "s2/s2cell.h"
#include "s2/s2latlng.h"

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

class S2CellOperatorException: public std::runtime_error {
public:
  S2CellOperatorException(std::string msg): std::runtime_error(msg.c_str()) {}
};

template<class VectorType, class ScalarType>
class UnaryS2CellOperator {
public:
  VectorType processVector(Rcpp::NumericVector cellIdVector) {
    VectorType output(cellIdVector.size());

    for (R_xlen_t i = 0; i < cellIdVector.size(); i++) {
      if ((i % 1000) == 0) {
        Rcpp::checkUserInterrupt();
      }

      S2CellId cell(*((uint64_t*) &(cellIdVector[i])));
      output[i] = this->processCell(cell, i);
    }

    return output;
  }

  virtual ScalarType processCell(S2CellId cellId, R_xlen_t i) = 0;
};

// For speed, take care of recycling here (only works if there is no
// additional parameter). Most binary ops don't have a parameter and some
// (like Ops, and Math) make recycling harder to incorporate at the R level
template<class VectorType, class ScalarType>
class BinaryS2CellOperator {
public:
  VectorType processVector(Rcpp::NumericVector cellIdVector1,
                           Rcpp::NumericVector cellIdVector2) {

    if (cellIdVector1.size() == cellIdVector2.size()) {
      VectorType output(cellIdVector1.size());

      for (R_xlen_t i = 0; i < cellIdVector1.size(); i++) {
        if ((i % 1000) == 0) {
          Rcpp::checkUserInterrupt();
        }

        S2CellId cell1(*((uint64_t*) &(cellIdVector1[i])));
        S2CellId cell2(*((uint64_t*) &(cellIdVector2[i])));
        output[i] = this->processCell(cell1, cell2, i);
      }

      return output;
    } else if (cellIdVector1.size() == 1) {
      VectorType output(cellIdVector2.size());

      for (R_xlen_t i = 0; i < cellIdVector2.size(); i++) {
        if ((i % 1000) == 0) {
          Rcpp::checkUserInterrupt();
        }

        S2CellId cell1(*((uint64_t*) &(cellIdVector1[0])));
        S2CellId cell2(*((uint64_t*) &(cellIdVector2[i])));
        output[i] = this->processCell(cell1, cell2, i);
      }

      return output;
    } else if (cellIdVector2.size() == 1) {
      VectorType output(cellIdVector1.size());

      for (R_xlen_t i = 0; i < cellIdVector1.size(); i++) {
        if ((i % 1000) == 0) {
          Rcpp::checkUserInterrupt();
        }

        S2CellId cell1(*((uint64_t*) &(cellIdVector1[i])));
        S2CellId cell2(*((uint64_t*) &(cellIdVector2[0])));
        output[i] = this->processCell(cell1, cell2, i);
      }

      return output;
    } else {
      std::stringstream err;
      err << 
        "Can't recycle vectors of size " << cellIdVector1.size() << 
        " and " << cellIdVector2.size() <<
        " to a common length.";
      stop(err.str());
    }
  }

  virtual ScalarType processCell(S2CellId cellId1, S2CellId cellId2, R_xlen_t i) = 0;
};

// [[Rcpp::export]]
NumericVector cpp_s2_cell_sentinel() {
  NumericVector result = NumericVector::create(reinterpret_double(S2CellId::Sentinel().id()));
  result.attr("class") = CharacterVector::create("s2_cell", "wk_vctr");
  return result;
}

// [[Rcpp::export]]
NumericVector cpp_s2_cell_from_string(CharacterVector cellString) {
  R_xlen_t size = cellString.size();
  NumericVector cellId(size);
  double* ptrDouble = REAL(cellId);
  uint64_t* ptrCellId = (uint64_t*) ptrDouble;

  for (R_xlen_t i = 0; i < size; i++) {
    if ((i % 1000) == 0) {
      Rcpp::checkUserInterrupt();
    }

    if (CharacterVector::is_na(cellString[i])) {
      ptrDouble[i] = NA_REAL;
    } else {
      ptrCellId[i] = S2CellId::FromToken(as<std::string>(cellString[i])).id();
    }
  }
  
  cellId.attr("class") = CharacterVector::create("s2_cell", "wk_vctr");
  return cellId;
}

// [[Rcpp::export]]
NumericVector cpp_s2_cell_from_lnglat(List lnglat) {
    NumericVector lng = lnglat[0];
    NumericVector lat = lnglat[1];
    R_xlen_t size = lng.size();
    NumericVector cellId(size);
    double* ptrDouble = REAL(cellId);
    uint64_t* ptrCellId = (uint64_t*) ptrDouble;

    for (R_xlen_t i = 0; i < size; i++) {
      if ((i % 1000) == 0) {
        Rcpp::checkUserInterrupt();
      }

      if (R_IsNA(lng[i]) || R_IsNA(lat[i])) {
          ptrDouble[i] = NA_REAL;
      } else {
          S2LatLng ll = S2LatLng::FromDegrees(lat[i], lng[i]).Normalized();
          ptrCellId[i] = S2CellId(ll).id();
      }
    }

    cellId.attr("class") = CharacterVector::create("s2_cell", "wk_vctr");
    return cellId;
}

// [[Rcpp::export]]
List cpp_s2_cell_to_lnglat(NumericVector cellId) {
    R_xlen_t size = cellId.size();
    double* ptrDouble = REAL(cellId);
    uint64_t* ptrCellId = (uint64_t*) ptrDouble;

    NumericVector lng(size);
    NumericVector lat(size);

    for (R_xlen_t i = 0; i < size; i++) {
      if ((i % 1000) == 0) {
        Rcpp::checkUserInterrupt();
      }

      if (R_IsNA(ptrDouble[i])) {
          lng[i] = NA_REAL;
          lat[i] = NA_REAL;
      } else {
          S2CellId cell(ptrCellId[i]);
          if (!cell.is_valid()) {
            lng[i] = NA_REAL;
            lat[i] = NA_REAL;
          } else {
            S2LatLng ll = S2CellId(ptrCellId[i]).ToLatLng();
            lng[i] = ll.lng().degrees();
            lat[i] = ll.lat().degrees();
          }
      }
    }

    return List::create(_["x"] = lng, _["y"] = lat);
}

// [[Rcpp::export]]
LogicalVector cpp_s2_cell_is_na(NumericVector cellIdVector) {
  LogicalVector out(cellIdVector.size());
  for (R_xlen_t i = 0; i < cellIdVector.size(); i++) {
    out = R_IsNA(cellIdVector[i]);
  }
  return out;
}

// [[Rcpp::export]]
NumericVector cpp_s2_cell_sort(NumericVector cellIdVector, bool decreasing) {
  NumericVector out = clone(cellIdVector);
  uint64_t* data = (uint64_t*) REAL(out);

  if (decreasing) {
    std::sort(data, data + out.size(), std::greater<uint64_t>());
  } else {
    std::sort(data, data + out.size());
  }

  out.attr("class") = CharacterVector::create("s2_cell", "wk_vctr");
  return out;
}

// [[Rcpp::export]]
NumericVector cpp_s2_cell_range(NumericVector cellIdVector, bool naRm) {
  uint64_t* data = (uint64_t*) REAL(cellIdVector);
  uint64_t zero = 0;
  uint64_t big = ~zero;
  auto dataRange = std::pair<uint64_t, uint64_t>(big, zero);
  // without NA handling this is just
  // dataRange = std::minmax_element(data, data + cellIdVector.size());

  for (R_xlen_t i = 0; i < cellIdVector.size(); i++) {
    if (R_IsNA(cellIdVector[i]) && !naRm) {
      dataRange.first = data[i];
      dataRange.second = data[i];
      break;
    }
    
    if (!R_IsNA(cellIdVector[i]) && (data[i] < dataRange.first)) {
      dataRange.first = data[i];
    }
    
    if (!R_IsNA(cellIdVector[i]) && (data[i] > dataRange.second)) {
      dataRange.second = data[i];
    }
  }

  if (dataRange.first > dataRange.second) {
    NumericVector out = NumericVector::create(NA_REAL, NA_REAL);
    out.attr("class") = CharacterVector::create("s2_cell", "wk_vctr");
    return out;
  } else {
    NumericVector out = NumericVector::create(
      reinterpret_double(dataRange.first),
      reinterpret_double(dataRange.second)
    );
    out.attr("class") = CharacterVector::create("s2_cell", "wk_vctr");
    return out;
  }
}

// [[Rcpp::export]]
NumericVector cpp_s2_cell_unique(NumericVector cellIdVector) {
  std::set<uint64_t> uniqueValues;
  uint64_t value;
  for (R_xlen_t i = 0; i < cellIdVector.size(); i++) {
    memcpy(&value, &(cellIdVector[i]), sizeof(uint64_t));
    uniqueValues.insert(value);
  }

  NumericVector out(uniqueValues.size());
  R_xlen_t i = 0;
  for (uint64_t value : uniqueValues) {
    out[i++] = reinterpret_double(value);
  }

  out.attr("class") = CharacterVector::create("s2_cell", "wk_vctr");
  return out;
}

// [[Rcpp::export]]
CharacterVector cpp_s2_cell_to_string(NumericVector cellIdVector) {
  class Op: public UnaryS2CellOperator<CharacterVector, String> {
    String processCell(S2CellId cellId, R_xlen_t i) {
      if (R_IsNA(reinterpret_double(cellId.id()))) {
        return NA_STRING;
      } else {
        return cellId.ToToken();
      }
    }
  };

  Op op;
  return op.processVector(cellIdVector);
}

// [[Rcpp::export]]
CharacterVector cpp_s2_cell_debug_string(NumericVector cellIdVector) {
  class Op: public UnaryS2CellOperator<CharacterVector, String> {
    String processCell(S2CellId cellId, R_xlen_t i) {
      if (R_IsNA(reinterpret_double(cellId.id()))) {
        return NA_STRING;
      } else {
        return cellId.ToString();
      }
    }
  };

  Op op;
  return op.processVector(cellIdVector);
}

// [[Rcpp::export]]
LogicalVector cpp_s2_cell_is_valid(NumericVector cellIdVector) {
  class Op: public UnaryS2CellOperator<LogicalVector, int> {
    int processCell(S2CellId cellId, R_xlen_t i) {
      return cellId.is_valid();
    }
  };

  Op op;
  return op.processVector(cellIdVector);
}

// [[Rcpp::export]]
List cpp_s2_cell_center(NumericVector cellIdVector) {
  class Op: public UnaryS2CellOperator<List, SEXP> {
    SEXP processCell(S2CellId cellId, R_xlen_t i) {
      if (cellId.is_valid()) {
        return XPtr<PointGeography>(new PointGeography(cellId.ToPoint()));
      } else {
        return R_NilValue;
      }
    }
  };

  Op op;
  List result = op.processVector(cellIdVector);
  result.attr("class") = CharacterVector::create("s2_geography", "s2_xptr");
  return result;
}

// [[Rcpp::export]]
List cpp_s2_cell_polygon(NumericVector cellIdVector) {
  class Op: public UnaryS2CellOperator<List, SEXP> {
    SEXP processCell(S2CellId cellId, R_xlen_t i) {
      if (cellId.is_valid()) {
        return XPtr<PolygonGeography>(new PolygonGeography(absl::make_unique<S2Polygon>(S2Cell(cellId))));
      } else {
        return R_NilValue;
      }
    }
  };

  Op op;
  List result = op.processVector(cellIdVector);
  result.attr("class") = CharacterVector::create("s2_geography", "s2_xptr");
  return result;
}

// [[Rcpp::export]]
List cpp_s2_cell_vertex(NumericVector cellIdVector, IntegerVector k) {
  class Op: public UnaryS2CellOperator<List, SEXP> {
    SEXP processCell(S2CellId cellId, R_xlen_t i) {
      if (cellId.is_valid() && (this->k[i] >= 0)) {
        return XPtr<PointGeography>(new PointGeography(S2Cell(cellId).GetVertex(this->k[i])));
      } else {
        return R_NilValue;
      }
    }

  public:
    IntegerVector k;
  };

  Op op;
  op.k = k;
  List result = op.processVector(cellIdVector);
  result.attr("class") = CharacterVector::create("s2_geography", "s2_xptr");
  return result;
}

// [[Rcpp::export]]
IntegerVector cpp_s2_cell_level(NumericVector cellIdVector) {
  class Op: public UnaryS2CellOperator<IntegerVector, int> {
    int processCell(S2CellId cellId, R_xlen_t i) {
      if (cellId.is_valid()) {
        return cellId.level();
      } else {
        return NA_INTEGER;
      }
    }
  };

  Op op;
  return op.processVector(cellIdVector);
}

// [[Rcpp::export]]
NumericVector cpp_s2_cell_area(NumericVector cellIdVector) {
  class Op: public UnaryS2CellOperator<NumericVector, double> {
    double processCell(S2CellId cellId, R_xlen_t i) {
      if (cellId.is_valid()) {
        return S2Cell(cellId).ExactArea();
      } else {
        return NA_REAL;
      }
    }
  };

  Op op;
  return op.processVector(cellIdVector);
}

// [[Rcpp::export]]
NumericVector cpp_s2_cell_area_approx(NumericVector cellIdVector) {
  class Op: public UnaryS2CellOperator<NumericVector, double> {
    double processCell(S2CellId cellId, R_xlen_t i) {
      if (cellId.is_valid()) {
        return S2Cell(cellId).ApproxArea();
      } else {
        return NA_REAL;
      }
    }
  };

  Op op;
  return op.processVector(cellIdVector);
}

// [[Rcpp::export]]
NumericVector cpp_s2_cell_parent(NumericVector cellIdVector, IntegerVector level) {
  class Op: public UnaryS2CellOperator<NumericVector, double> {
    double processCell(S2CellId cellId, R_xlen_t i) {
      int leveli = this->level[i];

      // allow negative numbers to relate to current level
      if (leveli < 0) {
        leveli = cellId.level() + leveli;
      }

      if (cellId.is_valid() && (leveli >= 0) && (leveli <= cellId.level())) {
        return reinterpret_double(cellId.parent(leveli).id());
      } else {
        return NA_REAL;
      }
    }

  public:
    IntegerVector level;
  };

  Op op;
  op.level = level;
  NumericVector result = op.processVector(cellIdVector);
  result.attr("class") = CharacterVector::create("s2_cell", "wk_vctr");
  return result;
}

// [[Rcpp::export]]
NumericVector cpp_s2_cell_child(NumericVector cellIdVector, IntegerVector k) {
  class Op: public UnaryS2CellOperator<NumericVector, double> {
    double processCell(S2CellId cellId, R_xlen_t i) {
      int ki = this->k[i];
      if (cellId.is_valid() && (ki >=0) && (ki <= 3)) {
        return reinterpret_double(cellId.child(ki).id());
      } else {
        return NA_REAL;
      }
    }

  public:
    IntegerVector k;
  };

  Op op;
  op.k = k;
  NumericVector result = op.processVector(cellIdVector);
  result.attr("class") = CharacterVector::create("s2_cell", "wk_vctr");
  return result;
}

// [[Rcpp::export]]
NumericVector cpp_s2_cell_edge_neighbour(NumericVector cellIdVector, IntegerVector k) {
  class Op: public UnaryS2CellOperator<NumericVector, double> {
    double processCell(S2CellId cellId, R_xlen_t i) {
      int ki = this->k[i];
      if (cellId.is_valid() && (ki >=0) && (ki <= 3)) {
        S2CellId neighbours[4];
        cellId.GetEdgeNeighbors(neighbours);
        return reinterpret_double(neighbours[ki].id());
      } else {
        return NA_REAL;
      }
    }

  public:
    IntegerVector k;
  };

  Op op;
  op.k = k;
  NumericVector result = op.processVector(cellIdVector);
  result.attr("class") = CharacterVector::create("s2_cell", "wk_vctr");
  return result;
}

// Ops for Ops, Math, Summary generics

// [[Rcpp::export]]
NumericVector cpp_s2_cell_cummax(NumericVector cellIdVector) {
  class Op: public UnaryS2CellOperator<NumericVector, double> {
  public:
    Op(): current(reinterpret_double(0)), currentId(0) {}

    double processCell(S2CellId cellId, R_xlen_t i) {
      double doubleVal = reinterpret_double(cellId.id());
      if (R_IsNA(this->current) || R_IsNA(doubleVal)) {
        this->current = NA_REAL;
        this->currentId = cellId.id();
        return NA_REAL;
      } else if (cellId.id() > this->currentId) {
        this->currentId = cellId.id();
        this->current = doubleVal;
        return doubleVal;
      } else {
        return this->current;
      }
    }

    double current;
    uint64_t currentId;
  };

  Op op;
  NumericVector result = op.processVector(cellIdVector);
  result.attr("class") = CharacterVector::create("s2_cell", "wk_vctr");
  return result;
}

// [[Rcpp::export]]
NumericVector cpp_s2_cell_cummin(NumericVector cellIdVector) {
  class Op: public UnaryS2CellOperator<NumericVector, double> {
  public:
    Op(): current(reinterpret_double(UINT64_MAX)), currentId(UINT64_MAX) {}

    double processCell(S2CellId cellId, R_xlen_t i) {
      double doubleVal = reinterpret_double(cellId.id());
      if (R_IsNA(this->current) || R_IsNA(doubleVal)) {
        this->current = NA_REAL;
        this->currentId = cellId.id();
        return NA_REAL;
      } else if (cellId.id() < this->currentId) {
        this->currentId = cellId.id();
        this->current = doubleVal;
        return doubleVal;
      } else {
        return this->current;
      }
    }

    double current;
    uint64_t currentId;
  };

  Op op;
  NumericVector result = op.processVector(cellIdVector);
  result.attr("class") = CharacterVector::create("s2_cell", "wk_vctr");
  return result;
}

// These are unique in that invalid cells (e.g., Sentinel)
// should not return NA; but NA_real_ should

// [[Rcpp::export]]
LogicalVector cpp_s2_cell_eq(NumericVector cellIdVector1, NumericVector cellIdVector2) {
  class Op: public BinaryS2CellOperator<LogicalVector, int> {
    int processCell(S2CellId cellId1, S2CellId cellId2, R_xlen_t i) {
      if (R_IsNA(reinterpret_double(cellId1.id())) || 
            R_IsNA(reinterpret_double(cellId2.id()))) {
        return NA_LOGICAL;
      } else {
        return cellId1.id() == cellId2.id();
      }
    }
  };

  Op op;
  return op.processVector(cellIdVector1, cellIdVector2);
}

// [[Rcpp::export]]
LogicalVector cpp_s2_cell_neq(NumericVector cellIdVector1, NumericVector cellIdVector2) {
  class Op: public BinaryS2CellOperator<LogicalVector, int> {
    int processCell(S2CellId cellId1, S2CellId cellId2, R_xlen_t i) {
      if (R_IsNA(reinterpret_double(cellId1.id())) || 
            R_IsNA(reinterpret_double(cellId2.id()))) {
        return NA_LOGICAL;
      } else {
        return cellId1.id() != cellId2.id();
      }
    }
  };

  Op op;
  return op.processVector(cellIdVector1, cellIdVector2);
}

// [[Rcpp::export]]
LogicalVector cpp_s2_cell_lt(NumericVector cellIdVector1, NumericVector cellIdVector2) {
  class Op: public BinaryS2CellOperator<LogicalVector, int> {
    int processCell(S2CellId cellId1, S2CellId cellId2, R_xlen_t i) {
      if (R_IsNA(reinterpret_double(cellId1.id())) || 
            R_IsNA(reinterpret_double(cellId2.id()))) {
        return NA_LOGICAL;
      } else {
        return cellId1.id() < cellId2.id();
      }
    }
  };

  Op op;
  return op.processVector(cellIdVector1, cellIdVector2);
}

// [[Rcpp::export]]
LogicalVector cpp_s2_cell_lte(NumericVector cellIdVector1, NumericVector cellIdVector2) {
  class Op: public BinaryS2CellOperator<LogicalVector, int> {
    int processCell(S2CellId cellId1, S2CellId cellId2, R_xlen_t i) {
      if (R_IsNA(reinterpret_double(cellId1.id())) || 
            R_IsNA(reinterpret_double(cellId2.id()))) {
        return NA_LOGICAL;
      } else {
        return cellId1.id() <= cellId2.id();
      }
    }
  };

  Op op;
  return op.processVector(cellIdVector1, cellIdVector2);
}

// [[Rcpp::export]]
LogicalVector cpp_s2_cell_gte(NumericVector cellIdVector1, NumericVector cellIdVector2) {
  class Op: public BinaryS2CellOperator<LogicalVector, int> {
    int processCell(S2CellId cellId1, S2CellId cellId2, R_xlen_t i) {
      if (R_IsNA(reinterpret_double(cellId1.id())) || 
            R_IsNA(reinterpret_double(cellId2.id()))) {
        return NA_LOGICAL;
      } else {
        return cellId1.id() >= cellId2.id();
      }
    }
  };

  Op op;
  return op.processVector(cellIdVector1, cellIdVector2);
}

// [[Rcpp::export]]
LogicalVector cpp_s2_cell_gt(NumericVector cellIdVector1, NumericVector cellIdVector2) {
  class Op: public BinaryS2CellOperator<LogicalVector, int> {
    int processCell(S2CellId cellId1, S2CellId cellId2, R_xlen_t i) {
      if (R_IsNA(reinterpret_double(cellId1.id())) || 
            R_IsNA(reinterpret_double(cellId2.id()))) {
        return NA_LOGICAL;
      } else {
        return cellId1.id() > cellId2.id();
      }
    }
  };

  Op op;
  return op.processVector(cellIdVector1, cellIdVector2);
}

// [[Rcpp::export]]
LogicalVector cpp_s2_cell_contains(NumericVector cellIdVector1, NumericVector cellIdVector2) {
  class Op: public BinaryS2CellOperator<LogicalVector, int> {
    int processCell(S2CellId cellId1, S2CellId cellId2, R_xlen_t i) {
      if (cellId1.is_valid() && cellId2.is_valid()) {
        return cellId1.contains(cellId2);
      } else {
        return NA_LOGICAL;
      }
    }
  };

  Op op;
  return op.processVector(cellIdVector1, cellIdVector2);
}

// [[Rcpp::export]]
LogicalVector cpp_s2_cell_may_intersect(NumericVector cellIdVector1, NumericVector cellIdVector2) {
  class Op: public BinaryS2CellOperator<LogicalVector, int> {
    int processCell(S2CellId cellId1, S2CellId cellId2, R_xlen_t i) {
      if (cellId1.is_valid() && cellId2.is_valid()) {
        return S2Cell(cellId1).MayIntersect(S2Cell(cellId2));
      } else {
        return NA_LOGICAL;
      }
    }
  };

  Op op;
  return op.processVector(cellIdVector1, cellIdVector2);
}

// [[Rcpp::export]]
NumericVector cpp_s2_cell_distance(NumericVector cellIdVector1, NumericVector cellIdVector2) {
  class Op: public BinaryS2CellOperator<NumericVector, double> {
    double processCell(S2CellId cellId1, S2CellId cellId2, R_xlen_t i) {
      if (cellId1.is_valid() && cellId2.is_valid()) {
        return S2Cell(cellId1).GetDistance(S2Cell(cellId2)).radians();
      } else {
        return NA_REAL;
      }
    }
  };

  Op op;
  return op.processVector(cellIdVector1, cellIdVector2);
}

// [[Rcpp::export]]
NumericVector cpp_s2_cell_max_distance(NumericVector cellIdVector1, NumericVector cellIdVector2) {
  class Op: public BinaryS2CellOperator<NumericVector, double> {
    double processCell(S2CellId cellId1, S2CellId cellId2, R_xlen_t i) {
      if (cellId1.is_valid() && cellId2.is_valid()) {
        return S2Cell(cellId1).GetMaxDistance(S2Cell(cellId2)).radians();
      } else {
        return NA_REAL;
      }
    }
  };

  Op op;
  return op.processVector(cellIdVector1, cellIdVector2);
}
