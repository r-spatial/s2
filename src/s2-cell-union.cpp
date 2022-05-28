
#include "s2/s2cell_id.h"
#include "s2/s2cell.h"
#include "s2/s2latlng.h"
#include "s2/s2cell_union.h"
#include "s2/s2region_coverer.h"
#include "s2/s2shape_index_buffered_region.h"
#include "s2/s2region_union.h"

#include "geography-operator.h"

#include <Rcpp.h>
using namespace Rcpp;


static inline double reinterpret_double(uint64_t id) {
  double doppelganger;
  memcpy(&doppelganger, &id, sizeof(double));
  return doppelganger;
}

S2CellUnion cell_union_from_cell_id_vector(const NumericVector& cellIdNumeric) {
  uint64* cellIds = (uint64*) &(cellIdNumeric[0]);
  std::vector<S2CellId> cellIdsVector(cellIds, cellIds + cellIdNumeric.size());
  return S2CellUnion(std::move(cellIdsVector));
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
        S2CellUnion cellUnion = cell_union_from_cell_id_vector(item);
        output[i] = this->processCell(cellUnion, i);
      }
    }

    return output;
  }

  virtual ScalarType processCell(S2CellUnion& cellUnion, R_xlen_t i) = 0;
};

// For speed, take care of recycling here (only works if there is no
// additional parameter). Most binary ops don't have a parameter and some
// (like Ops, and Math) make recycling harder to incorporate at the R level
template<class VectorType, class ScalarType>
class BinaryS2CellUnionOperator {
public:
  VectorType processVector(Rcpp::List cellUnionVector1,
                           Rcpp::List cellUnionVector2) {

    SEXP item1 = R_NilValue;
    SEXP item2 = R_NilValue;

    if (cellUnionVector1.size() == cellUnionVector2.size()) {
      VectorType output(cellUnionVector1.size());

      for (R_xlen_t i = 0; i < cellUnionVector1.size(); i++) {
        if ((i % 1000) == 0) {
          Rcpp::checkUserInterrupt();
        }

        item1 = cellUnionVector1[i];
        item2 = cellUnionVector2[i];

        if (item1 == R_NilValue || item2 == R_NilValue) {
          output[i] = VectorType::get_na();
        } else {
          S2CellUnion cellUnion1 = cell_union_from_cell_id_vector(item1);
          S2CellUnion cellUnion2 = cell_union_from_cell_id_vector(item2);
          output[i] = this->processCell(cellUnion1, cellUnion2, i);
        }
      }

      return output;
    } else if (cellUnionVector1.size() == 1) {
      VectorType output(cellUnionVector2.size());

      item1 = cellUnionVector1[0];
      if (item1 == R_NilValue) {
        for (R_xlen_t i = 0; i < cellUnionVector2.size(); i++) {
          if ((i % 1000) == 0) {
            Rcpp::checkUserInterrupt();
          }
          output[i] = VectorType::get_na();
        }

        return output;
      }

      S2CellUnion cellUnion1 = cell_union_from_cell_id_vector(item1);

      for (R_xlen_t i = 0; i < cellUnionVector2.size(); i++) {
        if ((i % 1000) == 0) {
          Rcpp::checkUserInterrupt();
        }

        item2 = cellUnionVector2[i];
        if (item2 == R_NilValue) {
          output[i] = VectorType::get_na();
        } else {
          S2CellUnion cellUnion2 = cell_union_from_cell_id_vector(item2);
          output[i] = this->processCell(cellUnion1, cellUnion2, i);
        }
      }

      return output;
    } else if (cellUnionVector2.size() == 1) {
      VectorType output(cellUnionVector1.size());

      item2 = cellUnionVector2[0];
      if (item2 == R_NilValue) {
        for (R_xlen_t i = 0; i < cellUnionVector1.size(); i++) {
          if ((i % 1000) == 0) {
            Rcpp::checkUserInterrupt();
          }
          output[i] = VectorType::get_na();
        }

        return output;
      }

      S2CellUnion cellUnion2 = cell_union_from_cell_id_vector(item2);

      for (R_xlen_t i = 0; i < cellUnionVector1.size(); i++) {
        if ((i % 1000) == 0) {
          Rcpp::checkUserInterrupt();
        }

        item1 = cellUnionVector1[i];
        if (item1 == R_NilValue) {
          output[i] = VectorType::get_na();
        } else {
          S2CellUnion cellUnion1 = cell_union_from_cell_id_vector(item1);
          output[i] = this->processCell(cellUnion1, cellUnion2, i);
        }
      }

      return output;
    } else {
      std::stringstream err;
      err <<
        "Can't recycle vectors of size " << cellUnionVector1.size() <<
        " and " << cellUnionVector2.size() <<
        " to a common length.";
      stop(err.str());
    }
  }

  virtual ScalarType processCell(const S2CellUnion& cellUnion1, const S2CellUnion& cellUnion2, R_xlen_t i) = 0;
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
LogicalVector cpp_s2_cell_union_is_na(List cellUnionVector) {
  LogicalVector out(cellUnionVector.size());
  for (R_xlen_t i = 0; i < cellUnionVector.size(); i++) {
    SEXP item = cellUnionVector[i];
    out[i] = item == R_NilValue;
  }
  return out;
}

// [[Rcpp::export]]
LogicalVector cpp_s2_cell_union_contains(List cellUnionVector1, List cellUnionVector2) {
  class Op: public BinaryS2CellUnionOperator<LogicalVector, int> {
    int processCell(const S2CellUnion& cellUnion1, const S2CellUnion& cellUnion2, R_xlen_t i) {
      return cellUnion1.Contains(cellUnion2);
    }
  };

  Op op;
  return op.processVector(cellUnionVector1, cellUnionVector2);
}

// optimized because it's a common case
// [[Rcpp::export]]
LogicalVector cpp_s2_cell_union_contains_cell(List cellUnionVector, NumericVector cellIdVector) {
  class Op: public UnaryS2CellUnionOperator<LogicalVector, int> {
    double* cellIdDouble;
    R_xlen_t cellIdVectorSize;

  public:

    Op(NumericVector cellIdVector) {
      cellIdDouble = REAL(cellIdVector);
      cellIdVectorSize = cellIdVector.size();
    }

    int processCell(S2CellUnion& cellUnion, R_xlen_t i) {
      if (R_IsNA(cellIdDouble[i % cellIdVectorSize])) {
        return NA_LOGICAL;
      } else {
        S2CellId cellId(((uint64_t*) cellIdDouble)[i % cellIdVectorSize]);
        return cellUnion.Contains(cellId);
      }
    }
  };

  Op op(cellIdVector);
  return op.processVector(cellUnionVector);
}

// [[Rcpp::export]]
LogicalVector cpp_s2_cell_union_intersects(List cellUnionVector1, List cellUnionVector2) {
  class Op: public BinaryS2CellUnionOperator<LogicalVector, int> {
    int processCell(const S2CellUnion& cellUnion1, const S2CellUnion& cellUnion2, R_xlen_t i) {
      return cellUnion1.Intersects(cellUnion2);
    }
  };

  Op op;
  return op.processVector(cellUnionVector1, cellUnionVector2);
}

// [[Rcpp::export]]
List cpp_s2_cell_union_intersection(List cellUnionVector1, List cellUnionVector2) {
  class Op: public BinaryS2CellUnionOperator<List, SEXP> {
    SEXP processCell(const S2CellUnion& cellUnion1, const S2CellUnion& cellUnion2, R_xlen_t i) {
      return cell_id_vector_from_cell_union(cellUnion1.Intersection(cellUnion2));
    }
  };

  Op op;
  List out = op.processVector(cellUnionVector1, cellUnionVector2);
  out.attr("class") = CharacterVector::create("s2_cell_union", "wk_vctr");
  return out;
}

// [[Rcpp::export]]
List cpp_s2_cell_union_union(List cellUnionVector1, List cellUnionVector2) {
  class Op: public BinaryS2CellUnionOperator<List, SEXP> {
    SEXP processCell(const S2CellUnion& cellUnion1, const S2CellUnion& cellUnion2, R_xlen_t i) {
      return cell_id_vector_from_cell_union(cellUnion1.Union(cellUnion2));
    }
  };

  Op op;
  List out = op.processVector(cellUnionVector1, cellUnionVector2);
  out.attr("class") = CharacterVector::create("s2_cell_union", "wk_vctr");
  return out;
}

// [[Rcpp::export]]
List cpp_s2_cell_union_difference(List cellUnionVector1, List cellUnionVector2) {
  class Op: public BinaryS2CellUnionOperator<List, SEXP> {
    SEXP processCell(const S2CellUnion& cellUnion1, const S2CellUnion& cellUnion2, R_xlen_t i) {
      return cell_id_vector_from_cell_union(cellUnion1.Difference(cellUnion2));
    }
  };

  Op op;
  List out = op.processVector(cellUnionVector1, cellUnionVector2);
  out.attr("class") = CharacterVector::create("s2_cell_union", "wk_vctr");
  return out;
}

// [[Rcpp::export]]
List cpp_s2_geography_from_cell_union(List cellUnionVector) {
  class Op: public UnaryS2CellUnionOperator<List, SEXP> {
    SEXP processCell(S2CellUnion& cellUnion, R_xlen_t i) {
      std::unique_ptr<S2Polygon> polygon = absl::make_unique<S2Polygon>();
      polygon->InitToCellUnionBorder(cellUnion);
      return RGeography::MakeXPtr(RGeography::MakePolygon(std::move(polygon)));
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

    SEXP processFeature(XPtr<RGeography> feature, R_xlen_t i) {
      S2ShapeIndexBufferedRegion region;
      region.Init(&feature->Index().ShapeIndex(), S1ChordAngle::Radians(this->distance[i]));

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


// [[Rcpp::export]]
List cpp_s2_covering_cell_ids_agg(List geog, int min_level, int max_level,
                                  int max_cells, double buffer, bool interior, bool naRm) {
  S2RegionCoverer coverer;
  coverer.mutable_options()->set_min_level(min_level);
  coverer.mutable_options()->set_max_level(max_level);
  coverer.mutable_options()->set_max_cells(max_cells);
  S1ChordAngle bufferAngle = S1ChordAngle::Radians(buffer);

  S2RegionUnion regionUnion;
  SEXP item;
  for (R_xlen_t i = 0; i < geog.size(); i++) {
    item = geog[i];
    if (item == R_NilValue && !naRm) {
      List out = List::create(R_NilValue);
      out.attr("class") = CharacterVector::create("s2_cell_union", "wk_vctr");
      return out;
    }

    if (item != R_NilValue) {
      Rcpp::XPtr<RGeography> feature(item);
      auto region = absl::make_unique<S2ShapeIndexBufferedRegion>();
      region->Init(&feature->Index().ShapeIndex(), bufferAngle);
      regionUnion.Add(std::move(region));
    }
  }

  S2CellUnion covering;
  if (interior) {
    covering = coverer.GetInteriorCovering(regionUnion);
  } else {
    covering = coverer.GetCovering(regionUnion);
  }

  List out = List::create(cell_id_vector_from_cell_union(covering));
  out.attr("class") = CharacterVector::create("s2_cell_union", "wk_vctr");
  return out;
}
