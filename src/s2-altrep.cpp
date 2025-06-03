#define R_NO_REMAP
#include <R.h>
#include <Rinternals.h>
#include "R_ext/Altrep.h"

#include <Rcpp.h>
using namespace Rcpp;

R_altrep_class_t s2_geography_altrep_cls;

// [[Rcpp::export]]
SEXP new_s2_geography(SEXP data) {
  if (TYPEOF(data) != VECSXP) {
    Rf_error("s2_geography data must be a list");
  }

  SEXP obj = PROTECT(R_new_altrep(s2_geography_altrep_cls, data, R_NilValue));

  SEXP cls = PROTECT(Rf_allocVector(STRSXP, 2));
  SET_STRING_ELT(cls, 0, Rf_mkChar("s2_geography"));
  SET_STRING_ELT(cls, 1, Rf_mkChar("wk_vctr"));

  Rf_setAttrib(obj, R_ClassSymbol, cls);
  UNPROTECT(2);

  return obj;
}

// ALTREP implementation for s2_geography
static R_INLINE R_xlen_t s2_altrep_Length(SEXP obj) {
  SEXP data = R_altrep_data1(obj);
  return Rf_xlength(data);
}

static SEXP s2_altrep_Elt(SEXP obj, R_xlen_t i) {
  SEXP data = R_altrep_data1(obj);
  return VECTOR_ELT(data, i);
}

static SEXP s2_altrep_Serialized_state(SEXP obj)
{
  Function to_wkb = Environment::namespace_env("s2")["s2_as_binary"];

  return to_wkb(obj);
}

static SEXP s2_altrep_Unserialize(SEXP cls, SEXP state)
{
  Function from_wkb = Environment::namespace_env("s2")["s2_geog_from_wkb"];

  return from_wkb(state);
}

// [[Rcpp::init]]
void altrep_init(DllInfo *dll) {
  s2_geography_altrep_cls = R_make_altlist_class("s2_geography", "s2", dll);

  R_set_altrep_Length_method(s2_geography_altrep_cls, s2_altrep_Length);
  R_set_altlist_Elt_method(s2_geography_altrep_cls, s2_altrep_Elt);
  R_set_altrep_Serialized_state_method(s2_geography_altrep_cls, s2_altrep_Serialized_state);
  R_set_altrep_Unserialize_method(s2_geography_altrep_cls, s2_altrep_Unserialize);
};
