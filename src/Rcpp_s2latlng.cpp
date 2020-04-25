#include "s2/s2latlng.h"
#include "s2/s2point.h"
#include "s2/s2loop.h"
#include "s2/s2polygon.h"
#include "s2/s2shape.h"
#include <Rcpp.h>

using namespace Rcpp;

// Get a vector of `S2LatLng`s from a two column matrix from R.
std::vector<S2LatLng> S2LatLngVecFromR(NumericMatrix mat, int omit_last = 1){
  if(mat.ncol() != 2)
    stop("Can't interpret input as lat,lng - must be a two column matrix.");
  NumericVector lat = mat( _, 1); // long first, then lat
  NumericVector lng = mat( _, 0);
  const int n = lat.size();
  std::vector<S2LatLng> rslt(n - omit_last);
  for(int i = 0; i < n - omit_last; i++)
    rslt[i] = S2LatLng::FromDegrees(lat[i], lng[i]);
  return rslt;
} 

// Get a vector of `S2Point`s from a two column matrix from R.
std::vector<S2Point> S2PointVecFromR(NumericMatrix mat, int omit_last = 1){
  if(mat.ncol() != 2)
    stop("Can't interpret input as lat,lng - must be a two column matrix.");
  NumericVector lat = mat( _, 1); // long first, then lat
  NumericVector lng = mat( _, 0);
  const int n = lat.size();
  std::vector<S2Point> rslt(n - omit_last);
  for(int i = 0; i < n - omit_last; i++)
    rslt[i] = S2LatLng::FromDegrees(lat[i], lng[i]).ToPoint();
  return rslt;
}

// Wrap a vector of `S2LatLng`s to a two column matrix in R.
NumericMatrix S2LatLngVecToR(std::vector<S2LatLng> points){
  int n = points.size();
  NumericMatrix rslt(n,2);
  for(int i=0; i<n; i++){
    rslt(i,0) = points[i].lat().degrees();
    rslt(i,1) = points[i].lng().degrees();
  }
  return rslt;
}

//[[Rcpp::export]]
SEXP MakeS2Shape(List mat, bool oriented = false) {
	// S2Shape *s = new S2Shape();
	std::vector<std::unique_ptr<S2Loop> > loops(mat.size());
	for (int i = 0; i < mat.size(); i++) {
		NumericMatrix m = mat[i];
		S2Loop *l = new S2Loop(S2PointVecFromR(m));
		if (!oriented)
			l->Normalize();
		loops[i] = std::unique_ptr<S2Loop>(l);
	}
	S2Polygon* polygon = new S2Polygon;
	polygon->set_s2debug_override(S2Debug::DISABLE);
	if (oriented)
		polygon->InitOriented(std::move(loops));
	else
		polygon->InitNested(std::move(loops));
	return R_MakeExternalPtr((void *) polygon, R_NilValue, R_NilValue);
}

//[[Rcpp::export]]
List Intersects(List ptrs) {
	std::vector<S2Polygon *> p(ptrs.size());
	for (int i = 0; i < ptrs.size(); i++) {
		SEXP s = ptrs[i];
		p[i] = (S2Polygon *) R_ExternalPtrAddr(s);
	}
	for (int i = 0; i < ptrs.size(); i++) {
		IntegerVector ret(0);
		for (int j = 0; j < ptrs.size(); j++)
			if (p[i]->Intersects(p[j]))
				ret.push_back(j);
		ptrs[i] = ret;
	}
	return ptrs;
}

//[[Rcpp::export]]
List ReleaseShape(List ptrs) {
	for (int i = 0; i < ptrs.size(); i++) {
		SEXP s = ptrs[i];
		S2Polygon *p = (S2Polygon *) R_ExternalPtrAddr(s);
		// p->Release(); -->> boom!
		delete p;
	}
	return List::create();
}
