
#include "cpp-compat.h"
#include <Rcpp.h>
#include <Rmath.h>
using namespace Rcpp;

#include <stdarg.h> // va_ stuff

void cpp_compat_printf(const char* fmt, ...) {
  va_list args;
  va_start(args, fmt);
  Rprintf(fmt, args);
  va_end(args);
}

void cpp_compat_abort() {
  throw std::runtime_error("abort() called");
}

void cpp_compat_exit(int code) {
  throw std::runtime_error("exit() called");
}

int cpp_compat_random() {
  // trying to match what random() would return
  // www.gnu.org/software/libc/manual/html_node/BSD-Random.html#BSD-Random
  // the RNG state is correctly managed for functions that use
  // Rcpp::export...other functions will require management of the RNGScope
  return unif_rand() * INT_MAX;
}

void cpp_compat_srandom(int seed) {
  // pretty sure this should not have any effect
  // it gets called on load here with the initiation
  // of the Random class in s2testing, so it can't
  // error out
}

std::ostream& cpp_compat_cerr = Rcerr;
std::ostream& cpp_compat_cout = Rcout;
