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

static void s2_altrep_SetElt(SEXP obj, R_xlen_t i, SEXP v) {
  SEXP data = R_altrep_data1(obj);
  SET_VECTOR_ELT(data, i, v);
}

static void* s2_altrep_Dataptr(SEXP obj, Rboolean writable) {
  if (writable) Rf_error("unable to produce writable DATAPTR for list data");

  SEXP data = R_altrep_data1(obj);
  return (void*) DATAPTR_RO(data);
}

static const void* s2_altrep_Dataptr_or_null(SEXP obj) {
  return s2_altrep_Dataptr(obj, FALSE);
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
#endif

// [[Rcpp::export]]
SEXP make_s2_geography_altrep(SEXP list) {
#if defined(S2_GEOGRAPHY_ALTREP)
  return R_new_altrep(s2_geography_altrep_cls, list, R_NilValue);
#else
  // nothing to do
  return list;
#endif
}

void s2_init_altrep(DllInfo *dll) {
#if defined(S2_GEOGRAPHY_ALTREP)
  s2_geography_altrep_cls = R_make_altlist_class("s2_geography", "s2", dll);

  R_set_altrep_Length_method(s2_geography_altrep_cls, s2_altrep_Length);
  R_set_altlist_Elt_method(s2_geography_altrep_cls, s2_altrep_Elt);
  R_set_altlist_Set_elt_method(s2_geography_altrep_cls, s2_altrep_SetElt);
  R_set_altvec_Dataptr_method(s2_geography_altrep_cls, s2_altrep_Dataptr);
  R_set_altvec_Dataptr_or_null_method(s2_geography_altrep_cls, s2_altrep_Dataptr_or_null);
  R_set_altrep_Serialized_state_method(s2_geography_altrep_cls, s2_altrep_Serialized_state);
  R_set_altrep_Unserialize_method(s2_geography_altrep_cls, s2_altrep_Unserialize);
#endif
}
