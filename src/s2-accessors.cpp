
#include "geography-operator.h"
#include <Rcpp.h>
using namespace Rcpp;

// [[Rcpp::export]]
LogicalVector cpp_s2_is_collection(List geog) {
  class Op: public UnaryGeographyOperator<LogicalVector, int> {
    int processFeature(XPtr<RGeography> feature, R_xlen_t i) {
      return s2geography::s2_is_collection(feature->Geog());
    }
  };

  Op op;
  return op.processVector(geog);
}

// [[Rcpp::export]]
LogicalVector cpp_s2_is_valid(List geog) {
  class Op: public UnaryGeographyOperator<LogicalVector, int> {
    int processFeature(XPtr<RGeography> feature, R_xlen_t i) {
      return !s2geography::s2_find_validation_error(feature->Geog(), &error);
    }

    S2Error error;
  };

  Op op;
  return op.processVector(geog);
}

// [[Rcpp::export]]
CharacterVector cpp_s2_is_valid_reason(List geog) {
  class Op: public UnaryGeographyOperator<CharacterVector, String> {
    String processFeature(XPtr<RGeography> feature, R_xlen_t i) {
      if (s2geography::s2_find_validation_error(feature->Geog(), &error)) {
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
    int processFeature(XPtr<RGeography> feature, R_xlen_t i) {
      return s2geography::s2_dimension(feature->Geog());
    }
  };

  Op op;
  return op.processVector(geog);
}

// [[Rcpp::export]]
IntegerVector cpp_s2_num_points(List geog) {
  class Op: public UnaryGeographyOperator<IntegerVector, int> {
    int processFeature(XPtr<RGeography> feature, R_xlen_t i) {
      return s2geography::s2_num_points(feature->Geog());
    }
  };

  Op op;
  return op.processVector(geog);
}

// [[Rcpp::export]]
LogicalVector cpp_s2_is_empty(List geog) {
  class Op: public UnaryGeographyOperator<LogicalVector, int> {
    int processFeature(XPtr<RGeography> feature, R_xlen_t i) {
      return s2geography::s2_is_empty(feature->Geog());
    }
  };

  Op op;
  return op.processVector(geog);
}

// [[Rcpp::export]]
NumericVector cpp_s2_area(List geog) {
  class Op: public UnaryGeographyOperator<NumericVector, double> {
    double processFeature(XPtr<RGeography> feature, R_xlen_t i) {
      return s2geography::s2_area(feature->Geog());
    }
  };

  Op op;
  return op.processVector(geog);
}

// [[Rcpp::export]]
NumericVector cpp_s2_length(List geog) {
  class Op: public UnaryGeographyOperator<NumericVector, double> {
    double processFeature(XPtr<RGeography> feature, R_xlen_t i) {
      return s2geography::s2_length(feature->Geog());
    }
  };

  Op op;
  return op.processVector(geog);
}

// [[Rcpp::export]]
NumericVector cpp_s2_perimeter(List geog) {
  class Op: public UnaryGeographyOperator<NumericVector, double> {
    double processFeature(XPtr<RGeography> feature, R_xlen_t i) {
      return s2geography::s2_perimeter(feature->Geog());
    }
  };

  Op op;
  return op.processVector(geog);
}

// [[Rcpp::export]]
NumericVector cpp_s2_x(List geog) {
  class Op: public UnaryGeographyOperator<NumericVector, double> {
    double processFeature(XPtr<RGeography> feature, R_xlen_t i) {
      if (s2geography::s2_dimension(feature->Geog()) != 0) {
        Rcpp::stop("Can't compute X value of a non-point geography");
      }

      return s2geography::s2_x(feature->Geog());
    }
  };

  Op op;
  return op.processVector(geog);
}

// [[Rcpp::export]]
NumericVector cpp_s2_y(List geog) {
  class Op: public UnaryGeographyOperator<NumericVector, double> {
    double processFeature(XPtr<RGeography> feature, R_xlen_t i) {
      if (s2geography::s2_dimension(feature->Geog()) != 0) {
        Rcpp::stop("Can't compute Y value of a non-point geography");
      }

      return s2geography::s2_y(feature->Geog());
    }
  };

  Op op;
  return op.processVector(geog);
}

// [[Rcpp::export]]
NumericVector cpp_s2_project_normalized(List geog1, List geog2) {
  class Op: public BinaryGeographyOperator<NumericVector, double> {
    double processFeature(XPtr<RGeography> feature1,
                          XPtr<RGeography> feature2,
                          R_xlen_t i) {
      return s2geography::s2_project_normalized(feature1->Geog(), feature2->Geog());
    }
  };

  Op op;
  return op.processVector(geog1, geog2);
}

// [[Rcpp::export]]
NumericVector cpp_s2_distance(List geog1, List geog2) {
  class Op: public BinaryGeographyOperator<NumericVector, double> {

    double processFeature(XPtr<RGeography> feature1,
                          XPtr<RGeography> feature2,
                          R_xlen_t i) {
      double distance = s2geography::s2_distance(feature1->Index(), feature2->Index());

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

    double processFeature(XPtr<RGeography> feature1,
                          XPtr<RGeography> feature2,
                          R_xlen_t i) {
      double distance = s2geography::s2_max_distance(feature1->Index(), feature2->Index());

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
