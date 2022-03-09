
#include "geography-operator.h"
#include <Rcpp.h>
using namespace Rcpp;

#include "s2-geography/s2-geography.hpp"

// [[Rcpp::export]]
LogicalVector cpp_s2_is_collection(List geog) {
  class Op: public UnaryGeographyOperator<LogicalVector, int> {
    int processFeature(XPtr<Geography> feature, R_xlen_t i) {
      auto geog = feature->NewGeography();
      return s2geography::s2_is_collection(*geog);
    }
  };

  Op op;
  return op.processVector(geog);
}

// [[Rcpp::export]]
LogicalVector cpp_s2_is_valid(List geog) {
  class Op: public UnaryGeographyOperator<LogicalVector, int> {
    int processFeature(XPtr<Geography> feature, R_xlen_t i) {
      auto geog = feature->NewGeography();
      return !s2geography::s2_find_validation_error(*geog, &error);
    }

    S2Error error;
  };

  Op op;
  return op.processVector(geog);
}

// [[Rcpp::export]]
CharacterVector cpp_s2_is_valid_reason(List geog) {
  class Op: public UnaryGeographyOperator<CharacterVector, String> {
    String processFeature(XPtr<Geography> feature, R_xlen_t i) {
      auto geog = feature->NewGeography();
      if (s2geography::s2_find_validation_error(*geog, &error)) {
        return this->error.text();
      } else {
        return NA_STRING;
      }
    }

    S2Error error;
  };

  Op op;
  return op.processVector(geog);
}

// [[Rcpp::export]]
IntegerVector cpp_s2_dimension(List geog) {
  class Op: public UnaryGeographyOperator<IntegerVector, int> {
    int processFeature(XPtr<Geography> feature, R_xlen_t i) {
      auto geog = feature->NewGeography();
      return s2geography::s2_dimension(*geog);
    }
  };

  Op op;
  return op.processVector(geog);
}

// [[Rcpp::export]]
IntegerVector cpp_s2_num_points(List geog) {
  class Op: public UnaryGeographyOperator<IntegerVector, int> {
    int processFeature(XPtr<Geography> feature, R_xlen_t i) {
      auto geog = feature->NewGeography();
      return s2geography::s2_num_points(*geog);
    }
  };

  Op op;
  return op.processVector(geog);
}

// [[Rcpp::export]]
LogicalVector cpp_s2_is_empty(List geog) {
  class Op: public UnaryGeographyOperator<LogicalVector, int> {
    int processFeature(XPtr<Geography> feature, R_xlen_t i) {
      auto geog = feature->NewGeography();
      return s2geography::s2_is_empty(*geog);
    }
  };

  Op op;
  return op.processVector(geog);
}

// [[Rcpp::export]]
NumericVector cpp_s2_area(List geog) {
  class Op: public UnaryGeographyOperator<NumericVector, double> {
    double processFeature(XPtr<Geography> feature, R_xlen_t i) {
      auto geog = feature->NewGeography();
      return s2geography::s2_area(*geog);
    }
  };

  Op op;
  return op.processVector(geog);
}

// [[Rcpp::export]]
NumericVector cpp_s2_length(List geog) {
  class Op: public UnaryGeographyOperator<NumericVector, double> {
    double processFeature(XPtr<Geography> feature, R_xlen_t i) {
      auto geog = feature->NewGeography();
      return s2geography::s2_length(*geog);
    }
  };

  Op op;
  return op.processVector(geog);
}

// [[Rcpp::export]]
NumericVector cpp_s2_perimeter(List geog) {
  class Op: public UnaryGeographyOperator<NumericVector, double> {
    double processFeature(XPtr<Geography> feature, R_xlen_t i) {
      auto geog = feature->NewGeography();
      return s2geography::s2_perimeter(*geog);
    }
  };

  Op op;
  return op.processVector(geog);
}

// [[Rcpp::export]]
NumericVector cpp_s2_x(List geog) {
  class Op: public UnaryGeographyOperator<NumericVector, double> {
    double processFeature(XPtr<Geography> feature, R_xlen_t i) {
      auto geog = feature->NewGeography();
      if (s2geography::s2_dimension(*geog) != 0) {
        Rcpp::stop("Can't compute X value of a non-point geography");
      }

      return s2geography::s2_x(*geog);
    }
  };

  Op op;
  return op.processVector(geog);
}

// [[Rcpp::export]]
NumericVector cpp_s2_y(List geog) {
  class Op: public UnaryGeographyOperator<NumericVector, double> {
    double processFeature(XPtr<Geography> feature, R_xlen_t i) {
      auto geog = feature->NewGeography();
      if (s2geography::s2_dimension(*geog) != 0) {
        Rcpp::stop("Can't compute Y value of a non-point geography");
      }

      return s2geography::s2_y(*geog);
    }
  };

  Op op;
  return op.processVector(geog);
}

// [[Rcpp::export]]
NumericVector cpp_s2_project_normalized(List geog1, List geog2) {
  class Op: public BinaryGeographyOperator<NumericVector, double> {
    double processFeature(XPtr<Geography> feature1,
                          XPtr<Geography> feature2,
                          R_xlen_t i) {
      auto geog1 = feature1->NewGeography();
      auto geog2 = feature2->NewGeography();
      return s2geography::s2_project_normalized(*geog1, *geog2);
    }
  };

  Op op;
  return op.processVector(geog1, geog2);
}

// [[Rcpp::export]]
NumericVector cpp_s2_distance(List geog1, List geog2) {
  class Op: public BinaryGeographyOperator<NumericVector, double> {

    double processFeature(XPtr<Geography> feature1,
                          XPtr<Geography> feature2,
                          R_xlen_t i) {
      auto geog1 = feature1->NewGeography();
      auto geog2 = feature2->NewGeography();

      s2geography::S2GeographyShapeIndex index1(*geog1);
      s2geography::S2GeographyShapeIndex index2(*geog2);

      double distance = s2geography::s2_distance(index1, index2);

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
NumericVector cpp_s2_max_distance(List geog1, List geog2) {
  class Op: public BinaryGeographyOperator<NumericVector, double> {

    double processFeature(XPtr<Geography> feature1,
                          XPtr<Geography> feature2,
                          R_xlen_t i) {
      auto geog1 = feature1->NewGeography();
      auto geog2 = feature2->NewGeography();

      s2geography::S2GeographyShapeIndex index1(*geog1);
      s2geography::S2GeographyShapeIndex index2(*geog2);

      double distance = s2geography::s2_max_distance(index1, index2);

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
