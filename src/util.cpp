#define R_NO_REMAP
#include <R.h>
#include <Rinternals.h>

#include "util.h"

SEXP s2_ns_pkg = NULL;

void s2_init_cached_sexps(void) {
  // package namespace environment
  s2_ns_pkg = PROTECT(R_FindNamespace(PROTECT(Rf_mkString("nanoarrow"))));

  // mark the cached objects as in use to prevent deallocation
  R_PreserveObject(s2_ns_pkg);

  UNPROTECT(2);
}
