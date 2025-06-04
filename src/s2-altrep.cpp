#define R_NO_REMAP
#include <R.h>
#include <Rinternals.h>
#include <Rversion.h>

// ALTREP VECSXP are supported starting from R 4.3.0
//
// When compiling for an earlier target serialization support is disabled
#if defined(R_VERSION) && R_VERSION >= R_Version(4, 3, 0)
#include "R_ext/Altrep.h"
#define S2_GEOGRAPHY_ALTREP
#endif

#include <Rcpp.h>
using namespace Rcpp;

// ALTREP implementation for s2_geography
#if defined(S2_GEOGRAPHY_ALTREP)
R_altrep_class_t s2_geography_altrep_cls;

static R_xlen_t s2_altrep_Length(SEXP obj) {
  SEXP data = R_altrep_data1(obj);
  return Rf_xlength(data);
}

static SEXP s2_altrep_Elt(SEXP obj, R_xlen_t i) {
  SEXP data = R_altrep_data1(obj);
  return VECTOR_ELT(data, i);
}

static SEXP get_s2_namespace_env() {
  static SEXP env = NULL;

  if (env == NULL) {
    env = R_FindNamespace(PROTECT(Rf_mkString("s2")));
    UNPROTECT(1);
  }

  return env;
}

static SEXP s2_altrep_Serialized_state(SEXP obj) {
  // fetch the pointer to s2::s2_geography_serialize()
  SEXP env = get_s2_namespace_env();
  SEXP fn = Rf_findFun(Rf_install("s2_geography_serialize"), env);

  SEXP call = PROTECT(Rf_lang2(fn, obj));
  SEXP out = Rf_eval(call, env);

  UNPROTECT(1);
  return out;
}

static SEXP s2_altrep_Unserialize(SEXP cls, SEXP state) {
  // fetch the pointer to s2::s2_geography_unserialize()
  SEXP env = get_s2_namespace_env();
  SEXP fn = Rf_findFun(Rf_install("s2_geography_unserialize"), env);

  SEXP call = PROTECT(Rf_lang2(fn, state));
  SEXP out = Rf_eval(call, env);

  UNPROTECT(1);
  return out;
}
#endif

// [[Rcpp::export]]
SEXP new_s2_geography(SEXP data) {
  if (TYPEOF(data) != VECSXP) {
    Rf_error("s2_geography data must be a list");
  }

#if defined(S2_GEOGRAPHY_ALTREP)
  SEXP obj = PROTECT(R_new_altrep(s2_geography_altrep_cls, data, R_NilValue));
#else
  SEXP obj = data;
#endif

  SEXP cls = PROTECT(Rf_allocVector(STRSXP, 2));
  SET_STRING_ELT(cls, 0, Rf_mkChar("s2_geography"));
  SET_STRING_ELT(cls, 1, Rf_mkChar("wk_vctr"));

  Rf_setAttrib(obj, R_ClassSymbol, cls);
#if defined(S2_GEOGRAPHY_ALTREP)
  UNPROTECT(2);
#else
  UNPROTECT(1);
#endif
  return obj;
}

// [[Rcpp::init]]
void altrep_init(DllInfo *dll) {
#if defined(S2_GEOGRAPHY_ALTREP)
  s2_geography_altrep_cls = R_make_altlist_class("s2_geography", "s2", dll);

  R_set_altrep_Length_method(s2_geography_altrep_cls, s2_altrep_Length);
  R_set_altlist_Elt_method(s2_geography_altrep_cls, s2_altrep_Elt);
  R_set_altrep_Serialized_state_method(s2_geography_altrep_cls, s2_altrep_Serialized_state);
  R_set_altrep_Unserialize_method(s2_geography_altrep_cls, s2_altrep_Unserialize);
#endif
};
