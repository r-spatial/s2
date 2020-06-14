#include <Rcpp.h>
#include "snap.h"
using namespace Rcpp;

int snap_level = -1; // global variable

//' Set snap level for polygons and polylines
//' 
//' @param snap integer, snap level (max. 30, negative prevents snapping)
//' @name snaplevel
//' @returns s2_set_snaplevel returns the old value of snap level
//' @export
// [[Rcpp::export]]
IntegerVector s2_set_snaplevel(int snap = -1) {
	if (snap > 30 || snap < -1)
		stop("maximum snap level is 30, set to -1 to prevent snapping");
	int old_snap_level = snap_level;
	snap_level = snap;
	IntegerVector ret(1);
	ret[0] = old_snap_level;
	return ret;
}

//' @param snap integer, snap level (max. 30, negative prevents snapping)
//' @name snaplevel
//' @export
// [[Rcpp::export]]
IntegerVector s2_get_snaplevel(int snap = -1) {
	IntegerVector ret(1);
	ret[0] = snap_level;
	return ret;
}
