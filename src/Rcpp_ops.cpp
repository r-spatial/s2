#include "s2/s2polygon.h"
#include <Rcpp.h>

using namespace Rcpp;

//' Geometry operators for s2 geometries
//' 
//' @param x list with S2Polygons pointers
//' @param y list with S2Polygons pointers
//' @name s2ops
//' @export
//[[Rcpp::export]]
List s2Intersects(List x, List y, bool polygons = true) {
	if (polygons) {
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
					ret.push_back(j+1); // R: 1-based index
			x[i] = ret;
		}
	} else {
		std::vector<S2Polyline *> xp(x.size());
		for (int i = 0; i < x.size(); i++) {
			SEXP s = x[i];
			xp[i] = (S2Polyline *) R_ExternalPtrAddr(s);
		}
		std::vector<S2Polyline *> yp(y.size());
		for (int i = 0; i < y.size(); i++) {
			SEXP s = y[i];
			yp[i] = (S2Polyline *) R_ExternalPtrAddr(s);
		}
		for (int i = 0; i < x.size(); i++) {
			IntegerVector ret(0);
			for (int j = 0; j < y.size(); j++)
				if (xp[i]->Intersects(yp[j]))
					ret.push_back(j+1); // R: 1-based index
			x[i] = ret;
		}
	}
	return x;
}

//' @export
//' @name s2ops
//[[Rcpp::export]]
LogicalVector s2IsValid(List ptrs, bool polygon = true) {
	LogicalVector ret(ptrs.size());
	for (int i = 0; i < ptrs.size(); i++) {
		SEXP s = ptrs[i];
		if (polygon) {
			S2Polygon *p = (S2Polygon *) R_ExternalPtrAddr(s);
			ret[i] = p->IsValid();
		} else {
			S2Polyline *l = (S2Polyline *) R_ExternalPtrAddr(s);
			ret[i] = l->IsValid();
		}
	}
	return ret;
}

//' @export
//' @name s2ops
//[[Rcpp::export]]
NumericVector s2GetArea(List ptrs) {
	NumericVector ret(ptrs.size());
	for (int i = 0; i < ptrs.size(); i++) {
		SEXP s = ptrs[i];
		S2Polygon *p = (S2Polygon *) R_ExternalPtrAddr(s);
		ret[i] = p->GetArea();
	}
	return ret;
}
