
#ifndef GEOGRAPHY_OPERATOR_H
#define GEOGRAPHY_OPERATOR_H

#include <stdexcept>

#include "geography.h"
#include <Rcpp.h>

class GeographyOperatorException: public std::runtime_error {
public:
  GeographyOperatorException(std::string msg): std::runtime_error(msg.c_str()) {}
};

template<class VectorType, class ScalarType>
class UnaryGeographyOperator {
public:
  VectorType processVector(Rcpp::List geog) {
    VectorType output(geog.size());

    Rcpp::IntegerVector problemId;
    Rcpp::CharacterVector problems;

    SEXP item;
    for (R_xlen_t i = 0; i < geog.size(); i++) {
      Rcpp::checkUserInterrupt();

      item = geog[i];
      if (item == R_NilValue) {
        output[i] = VectorType::get_na();
      } else {
        Rcpp::XPtr<Geography> feature(item);

        try {
          output[i] = this->processFeature(feature, i);
        } catch (GeographyOperatorException& e) {
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

  virtual ScalarType processFeature(Rcpp::XPtr<Geography> feature, R_xlen_t i) = 0;
};


template<class VectorType, class ScalarType>
class BinaryGeographyOperator {
public:
  VectorType processVector(Rcpp::List geog1, Rcpp::List geog2) {
    if (geog2.size() != geog1.size()) {
      Rcpp::stop("Incompatible lengths");
    }

    VectorType output(geog1.size());

    Rcpp::IntegerVector problemId;
    Rcpp::CharacterVector problems;

    SEXP item1;
    SEXP item2;

    for (R_xlen_t i = 0; i < geog1.size(); i++) {
      Rcpp::checkUserInterrupt();
      
      item1 = geog1[i];
      item2 = geog2[i];
      if (item1 ==  R_NilValue || item2 == R_NilValue) {
        output[i] = VectorType::get_na();
      } else {
        Rcpp::XPtr<Geography> feature1(item1);
        Rcpp::XPtr<Geography> feature2(item2);

        try {
          output[i] = processFeature(feature1, feature2, i);
        } catch (GeographyOperatorException& e) {
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

  virtual ScalarType processFeature(Rcpp::XPtr<Geography> feature1,
                                    Rcpp::XPtr<Geography> feature2,
                                    R_xlen_t i) = 0;
};

#endif
