
#include <cstdint>
#include <vector>

#include "s2/s2cell_id.h"
#include "s2/s2cell.h"
#include "s2/s2latlng.h"

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

    Rcpp::IntegerVector problemId;
    Rcpp::CharacterVector problems;

    for (R_xlen_t i = 0; i < cellIdVector.size(); i++) {
      if ((i % 1000) == 0) {
        Rcpp::checkUserInterrupt();
      }

      if (NumericVector::is_na(cellIdVector[i])) {
        output[i] = VectorType::get_na();
      } else {
        try {
          S2CellId cell(*((uint64_t*) &(cellIdVector[i])));
          output[i] = this->processCell(cell, i);
        } catch (S2CellOperatorException& e) {
          output[i] = VectorType::get_na();
          problemId.push_back(i);
          problems.push_back(e.what());
        }
      }
    }

    if (problemId.size() > 0) {
      Rcpp::Environment s2NS = Rcpp::Environment::namespace_env("s2");
      Rcpp::Function stopProblems = s2NS["stop_problems_process"];
      stopProblems(problemId, problems);
    }

    return output;
  }

  virtual ScalarType processCell(S2CellId cellId, R_xlen_t i) = 0;
};

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

      if (NumericVector::is_na(lng[i]) || NumericVector::is_na(lat[i])) {
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

      if (NumericVector::is_na(ptrDouble[i])) {
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
CharacterVector cpp_s2_cell_to_string(NumericVector cellIdVector) {
  class Op: public UnaryS2CellOperator<CharacterVector, std::string> {
    std::string processCell(S2CellId cellId, R_xlen_t i) {
      return cellId.ToToken();
    }
  };

  Op op;
  return op.processVector(cellIdVector);
}

// [[Rcpp::export]]
CharacterVector cpp_s2_cell_to_debug_string(NumericVector cellIdVector) {
  class Op: public UnaryS2CellOperator<CharacterVector, std::string> {
    std::string processCell(S2CellId cellId, R_xlen_t i) {
      return cellId.ToString();
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
      return R_NilValue;
    }
  };

  Op op;
  return op.processVector(cellIdVector);
}

// [[Rcpp::export]]
List cpp_s2_cell_boundary(NumericVector cellIdVector) {
  class Op: public UnaryS2CellOperator<List, SEXP> {
    SEXP processCell(S2CellId cellId, R_xlen_t i) {
      if (cellId.is_valid()) {
        return R_NilValue;
      } else {
        return R_NilValue;
      }
    }
  };

  Op op;
  return op.processVector(cellIdVector);
}

// [[Rcpp::export]]
List cpp_s2_cell_polygon(NumericVector cellIdVector) {
  class Op: public UnaryS2CellOperator<List, SEXP> {
    SEXP processCell(S2CellId cellId, R_xlen_t i) {
      if (cellId.is_valid()) {
        return R_NilValue;
      } else {
        return R_NilValue;
      }
    }
  };

  Op op;
  return op.processVector(cellIdVector);
}

// [[Rcpp::export]]
List cpp_s2_cell_vertices(NumericVector cellIdVector) {
  class Op: public UnaryS2CellOperator<List, SEXP> {
    SEXP processCell(S2CellId cellId, R_xlen_t i) {
      if (cellId.is_valid()) {
        return R_NilValue;
      } else {
        return R_NilValue;
      }
    }
  };

  Op op;
  return op.processVector(cellIdVector);
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
      if (cellId.is_valid() && (leveli >= 0) && (leveli < cellId.level())) {
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
NumericVector cpp_s2_cell_area_edge_neighbour(NumericVector cellIdVector, IntegerVector k) {
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
