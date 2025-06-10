#ifndef S2_ALTREP_H
#define S2_ALTREP_H

#include <Rcpp.h>
#include <Rversion.h>

// ALTREP VECSXP are supported starting from R 4.3.0
//
// When compiling for an earlier target serialization support is disabled
#if defined(R_VERSION) && R_VERSION >= R_Version(4, 3, 0)
#define S2_GEOGRAPHY_ALTREP
#endif

void s2_init_altrep(DllInfo *dll);

#endif
