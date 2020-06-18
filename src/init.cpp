
#include "s2/s2debug.h"
#include <Rcpp.h>
using namespace Rcpp;

// [[Rcpp::export]]
void cpp_s2_init() {
  // It's important to set this flag, as users might have "debug" flags
  // for their build environment, and there are some checks that will terminate
  // R instead of throw an exception if this value is set to true.
  // When possible, we also disable debug checks on a per-operation basis
  // if there is another way to do so (e.g., constructing S2Loop and S2Polygon objects).
  FLAGS_s2debug = false; // # nocov
}
