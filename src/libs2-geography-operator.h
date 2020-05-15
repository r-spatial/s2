
#ifndef LIBS2_GEOGRAHPY_OPERATOR_H
#define LIBS2_GEOGRAHPY_OPERATOR_H

#include "libs2-s2geography.h"
#include <Rcpp.h>

template<class VectorType, class ScalarType>
class LibS2UnaryGeographyOperator {
public:
  VectorType processVector(Rcpp::List geog) {
    VectorType output(geog.size());

    SEXP item;
    for (R_xlen_t i = 0; i < geog.size(); i++) {
      item = geog[i];
      if (item == R_NilValue) {
        output[i] = VectorType::get_na();
      } else {
        Rcpp::XPtr<LibS2Geography> feature(item);
        output[i] = this->processFeature(feature);
      }
    }

    return output;
  }

  virtual ScalarType processFeature(Rcpp::XPtr<LibS2Geography> feature) = 0;
};


template<class VectorType, class ScalarType>
class LibS2BinaryGeographyOperator {
public:
  VectorType processVector(Rcpp::List geog1, Rcpp::List geog2) {
    if (geog2.size() != geog1.size()) {
      Rcpp::stop("Incompatible lengths");
    }

    VectorType output(geog1.size());

    SEXP item1;
    SEXP item2;

    for (R_xlen_t i = 0; i < geog1.size(); i++) {
      item1 = geog1[i];
      item2 = geog2[i];
      if (item1 ==  R_NilValue || item2 == R_NilValue) {
        output[i] = NA_REAL;
      } else {
        Rcpp::XPtr<LibS2Geography> feature1(item1);
        Rcpp::XPtr<LibS2Geography> feature2(item2);
        output[i] = processFeature(feature1, feature2);
      }
    }

    return output;
  }

  virtual ScalarType processFeature(Rcpp::XPtr<LibS2Geography> feature1,
                                    Rcpp::XPtr<LibS2Geography> feature2) = 0;
};

#endif