#include "s2/s2polygon.h"
#include <Rcpp.h>

using namespace Rcpp;

//' Return indices of intersecting S2Polygons
//' 
//' @param x list with S2Polygons pointers
//' @param y list with S2Polygons pointers
//' @export
//[[Rcpp::export]]
List s2Intersects(List x, List y) {
	std::vector<S2Polygon *> xp(x.size());
	for (int i = 0; i < x.size(); i++) {
		SEXP s = x[i];
		xp[i] = (S2Polygon *) R_ExternalPtrAddr(s);
	}
	std::vector<S2Polygon *> yp(y.size());
	for (int i = 0; i < y.size(); i++) {
		SEXP s = y[i];
		yp[i] = (S2Polygon *) R_ExternalPtrAddr(s);
	}
	for (int i = 0; i < x.size(); i++) {
		IntegerVector ret(0);
		for (int j = 0; j < y.size(); j++)
			if (xp[i]->Intersects(yp[j]))
				ret.push_back(j);
		x[i] = ret;
	}
	return x;
}
