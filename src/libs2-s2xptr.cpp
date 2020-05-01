#include <Rcpp.h>
using namespace Rcpp;

class XPtrTest {
public:
  XPtrTest() {
    try {
      Rcout << "Allocating XPtrTest at " << this << "\n";
    } catch (std::exception& error) {

    }
  }

  ~XPtrTest() {
    try {
      Rcout << "Destroying XPtrTest at " << this << "\n";
    } catch (std::exception& error) {

    }
  }
};

// [[Rcpp::export]]
List s2xptr_test(R_xlen_t size) {
  List output(size);
  for (R_xlen_t i = 0; i < size; i++) {
    output[i] = XPtr<XPtrTest>(new XPtrTest());
  }
  return output;
}
