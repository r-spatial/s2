#include "s2/s2latlng.h"
#include "s2/s2point.h"
#include "s2/s2loop.h"
#include "s2/s2polygon.h"
#include "s2/s2shape.h"
#include <Rcpp.h>

using namespace Rcpp;

std::vector<S2LatLng> S2LatLngVecFromR(NumericMatrix mat, int omit_last = 1) {
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

//' convert R list of coordinate matrices (lon,lat) into S2Polygon ptr 
//' 
//' @param mat two-column matrix with longitude in first, latitude in second column
//' @param oriented logical; if TRUE, rings are guaranteed to be oriented (e.g. read
//' by \code{read_sf} using \code{check_ring_dir=TRUE}), meaning CCW exterior rings
//' and CW holes; if FALSE, rings are normalized and holes are deduced from degree of nesting.
//' @name s2makepolygons
//' @export
//[[Rcpp::export]]
SEXP s2MakePolygon(List mat, bool oriented = false) {
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

//' @export
//' @param ptrs list with S2Polygon pointers
//' @name s2makepolygons
//[[Rcpp::export]]
List s2ReleasePolygons(List ptrs) {
	for (int i = 0; i < ptrs.size(); i++) {
		SEXP s = ptrs[i];
		S2Polygon *p = (S2Polygon *) R_ExternalPtrAddr(s);
		// p->Release(); -->> boom!
		delete p;
	}
	return List::create();
}
