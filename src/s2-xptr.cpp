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

  void test() {
    Rcout << "test() on XPtrTest at " << this << "\n";
  }

  ~XPtrTest() {
    try {
      Rcout << "Destroying XPtrTest at " << this << "\n";
    } catch (std::exception& error) {

    }
  }
};

// [[Rcpp::export]]
List s2_xptr_test(R_xlen_t size) {
  List output(size);
  for (R_xlen_t i = 0; i < size; i++) {
    output[i] = XPtr<XPtrTest>(new XPtrTest());
  }
  return output;
}

// [[Rcpp::export]]
void s2_xptr_test_op(List s2_xptr_test) {
  SEXP item;
  for (R_xlen_t i = 0; i < s2_xptr_test.size(); i++) {
    item = s2_xptr_test[i];
    if (item == R_NilValue) {
      Rcout << "Item is NULL\n";
    } else  {
      // the general idea is to make sure that this operation doesn't copy
      // the pointer or cause it to get destroyed
      XPtr<XPtrTest> ptr(item);
      ptr->test();
    }
  }
}
