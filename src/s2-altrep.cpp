#define R_NO_REMAP
#include <R.h>
#include <Rinternals.h>
#include <Rversion.h>

#include "s2-altrep.h"
#include "util.h"

// ALTREP implementation for s2_geography
#if defined(S2_GEOGRAPHY_ALTREP)
#include "R_ext/Altrep.h"
R_altrep_class_t s2_geography_altrep_cls;


static R_xlen_t s2_altrep_Length(SEXP obj) {
  SEXP data = R_altrep_data1(obj);
  return Rf_xlength(data);
}

static SEXP s2_altrep_Elt(SEXP obj, R_xlen_t i) {
  SEXP data = R_altrep_data1(obj);
  return VECTOR_ELT(data, i);
}


static SEXP s2_altrep_Serialized_state(SEXP obj) {
  // fetch the pointer to s2::s2_geography_serialize()
  SEXP fn = Rf_findFun(Rf_install("s2_geography_serialize"), s2_ns_pkg);

  SEXP call = PROTECT(Rf_lang2(fn, obj));
  SEXP out = Rf_eval(call, s2_ns_pkg);

  UNPROTECT(1);
  return out;
}

static SEXP s2_altrep_Unserialize(SEXP cls, SEXP state) {
  // fetch the pointer to s2::s2_geography_unserialize()
  SEXP fn = Rf_findFun(Rf_install("s2_geography_unserialize"), s2_ns_pkg);

  SEXP call = PROTECT(Rf_lang2(fn, state));
  SEXP out = Rf_eval(call, s2_ns_pkg);

  UNPROTECT(1);
  return out;
}


static SEXP setting_s2_geography_class(SEXP x) {
  // use callee protection here to simplify the caller code
  x = PROTECT(x);

  SEXP cls = PROTECT(Rf_allocVector(STRSXP, 2));
  SET_STRING_ELT(cls, 0, Rf_mkChar("s2_geography"));
  SET_STRING_ELT(cls, 1, Rf_mkChar("wk_vctr"));

  Rf_setAttrib(x, R_ClassSymbol, cls);

  UNPROTECT(2);

  return x;
}

// [[Rcpp::export]]
SEXP new_s2_geography(SEXP data) {
  if (TYPEOF(data) != VECSXP) {
    Rf_error("s2_geography data must be a list");
  }

#if defined(S2_GEOGRAPHY_ALTREP)
  if (!R_isTRUE(Rf_GetOption1(Rf_install("s2.disable_altrep")))) {
    // no protection is needed here since setting_s2_geography_class() protects its arguments
    data = R_new_altrep(s2_geography_altrep_cls, data, R_NilValue);
  }
#endif

  return setting_s2_geography_class(data);
}

void s2_init_altrep(DllInfo *dll) {
#if defined(S2_GEOGRAPHY_ALTREP)
  s2_geography_altrep_cls = R_make_altlist_class("s2_geography", "s2", dll);

  R_set_altrep_Length_method(s2_geography_altrep_cls, s2_altrep_Length);
  R_set_altlist_Elt_method(s2_geography_altrep_cls, s2_altrep_Elt);
  R_set_altrep_Serialized_state_method(s2_geography_altrep_cls, s2_altrep_Serialized_state);
  R_set_altrep_Unserialize_method(s2_geography_altrep_cls, s2_altrep_Unserialize);
#endif
};
#endif
