#include "s2/s1angle.h"
#include "s2/s2latlng.h"
#include "s2/s2point.h"
#include "s2/s2loop.h"
#include "s2/s2polygon.h"
#include <Rcpp.h>

using namespace Rcpp;

/*
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
*/

// Convert R long-lat matrix into S2Point vector
std::vector<S2Point> S2PointVecFromR(NumericMatrix mat, int omit_last = 1) {
  if(mat.ncol() != 2)
    stop("Can't interpret input as lng,lat - must be a two column matrix.");
  NumericVector lng = mat( _, 0);
  NumericVector lat = mat( _, 1); // long first, then lat
  const int n = lat.size();
  std::vector<S2Point> rslt(n - omit_last);
  for(int i = 0; i < n - omit_last; i++) {
    rslt[i] = S2LatLng::FromDegrees(lat[i], lng[i]).ToPoint();
  }
  return rslt;
}

/*
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
*/


//
// 1. Point (POINT):
//
void release_point(SEXP p) {
	S2Point *point = (S2Point *) R_ExternalPtrAddr(p);
	delete point;
	return;
}

//' convert R vector with coordinates (lon,lat) into S2Point ptr 
//' 
//' @param pt numeric; length 2, longitude latitude
//' @name s2makepoint
//' @export
//[[Rcpp::export]]
SEXP s2MakePoint(NumericVector pt) {
	if(pt.size() != 2)
		stop("Can't interpret input as lng,lat - must be a size two vector.");
  	S2Point *pp = new S2Point;
	*pp = S2LatLng::FromDegrees(pt[1], pt[0]).ToPoint();
	SEXP p = R_MakeExternalPtr((void *) pp, R_NilValue, R_NilValue);
	R_RegisterCFinalizerEx(p, release_point, TRUE);
	return p;
}

NumericVector getPoint(S2Point *p) {
	NumericVector v(2);
	S2LatLng ll(*p);
	v(0) = ll.lng().degrees();
	v(1) = ll.lat().degrees();
	return v;
}

//' @export
//' @name s2makepoints
//' @param ptrs R list with external references (pointers) to S2Point objects
//[[Rcpp::export]]
List s2GetPoint(List ptrs) {
	List ret(ptrs.size());
	for (int i = 0; i < ptrs.size(); i++) {
		SEXP s = ptrs[i];
		ret(i) = getPoint((S2Point *) R_ExternalPtrAddr(s));
	}
	return ret;
}

//
// 2. Polyline (LINESTRING)
//
void release_polyline(SEXP p) {
	S2Polyline *polyline = (S2Polyline *) R_ExternalPtrAddr(p);
	delete polyline;
	return;
}

//' convert R matrix with coordinates (lon,lat) into S2Polyline ptr 
//' 
//' @param pts 2-column numeric matrix with lng,lat line vertices
//' @name s2makepolyline
//' @export
//[[Rcpp::export]]
SEXP s2MakePolyline(NumericMatrix pts) {
	if(pts.ncol() != 2)
		stop("Can't interpret input as lng,lat - must be a size two vector.");
	
  	S2Polyline *pl = new S2Polyline;
	pl->Init(S2PointVecFromR(pts, 0)); 
	SEXP p = R_MakeExternalPtr((void *) pl, R_NilValue, R_NilValue);
	R_RegisterCFinalizerEx(p, release_polyline, TRUE);
	return p;
}

NumericMatrix getPolyline(S2Polyline *pl) {
	NumericMatrix m(pl->num_vertices(), 2);
	for (int i = 0; i < pl->num_vertices(); i++) {
		S2LatLng ll(pl->vertex(i));
		m(i, 0) = ll.lng().degrees();
		m(i, 1) = ll.lat().degrees();
	}
	return m;
}

//' @export
//' @name s2makepolyline
//' @param ptrs R list with external references (pointers) to S2Polyline objects
//[[Rcpp::export]]
List s2GetPolyline(List ptrs) {
	List ret(ptrs.size());
	for (int i = 0; i < ptrs.size(); i++) {
		SEXP s = ptrs[i];
		ret(i) = getPolyline((S2Polyline *) R_ExternalPtrAddr(s));
	}
	return ret;
}

//
// 3. Polygon (MULTIPOLYGON)
//
void release_polygon(SEXP p) {
	S2Polygon *pol = (S2Polygon *) R_ExternalPtrAddr(p);
	delete pol;
	return;
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
	SEXP p = R_MakeExternalPtr((void *) polygon, R_NilValue, R_NilValue);
	R_RegisterCFinalizerEx(p, release_polygon, TRUE);
	return p;
}


NumericMatrix LoopToMatrix(S2Loop *lp, bool close = true) { // return long/lat
	int n = lp->num_vertices();
	if (close)
		n = n + 1;
	NumericMatrix m(n, 2);
	for (int i = 0; i < lp->num_vertices(); i++) {
		S2LatLng ll(lp->oriented_vertex(i));
		m(i, 0) = ll.lng().degrees();
		m(i, 1) = ll.lat().degrees();
	}
	if (close) {
		m(n - 1, 0) = m(0, 0);
		m(n - 1, 1) = m(0, 1);
	}
	return m;
}

List getPolygon(S2Polygon *p) {
	List l(p->num_loops());
	IntegerVector idx(l.size());
	for (int i = 0; i < l.size(); i++) {
		l[i] = LoopToMatrix(p->loop(i));
		idx[i] = p->GetParent(i);
	}
	if (l.size() == 0)
		return l; // empty
	// get nested rings kludge into flat simple feature form:
	IntegerVector outer_index(l.size());
	// we know the first one is outer:
	int n_outer = 1;
	outer_index[0] = 0;
	// go through all others:
	for (int i = 1; i < l.size(); i++) {
		if (idx[i] == -1) { // top-level outer ring:
			outer_index[i] = n_outer;
			n_outer++;
		} else {
			// possibly hole;
			int ii = idx[i];
			if (outer_index[ii] >= 0) // refers to an outer loop: hole
				outer_index[i] = -1 - outer_index[ii];
			else { // refers to a hole: must be outer loop
				outer_index[i] = n_outer;
				n_outer++;
			}
		}
	}
	List outer_rings(n_outer);
	IntegerVector n_rings(n_outer);
	for (int i = 0; i < n_outer; i++)
		n_rings[i] = 1; // at least one, the outer ring
	// count holes:
	for (int i = 0; i < l.size(); i++) {
		int oi = outer_index[i];
		if (oi < 0) { // hole:
			oi = -oi - 1;
			n_rings[oi] = n_rings[oi] + 1;
		}
	}

	// fill lists:
	List ret(n_outer);
	IntegerVector counter(n_outer);
	for (int i = 0; i < n_outer; i++)
		counter[i] = 0;
	for (int j = 0; j < n_outer; j++) {
		List lst(n_rings[j]);
		int k = 0;
		for (int i = 0; i < l.size(); i++) {
			int oi = outer_index[i];
			if (oi >= 0 && oi == j) {
				lst(k) = l(i);
				k++;
			} else if (oi < 0 && j == -oi - 1) {
				lst(k) = l(i);
				k++;
			}
		}
		if (k != n_rings[j]) {
			Rprintf("k: %d, nrj: %d\n", k, n_rings[j]);
			stop("Error!");
		}
		ret(j) = lst;
	}
	return ret;
}

//' @export
//' @name s2makepolygons
//' @param ptrs R list with external references (pointers) to S2Polygon objects
//[[Rcpp::export]]
List s2GetPolygon(List ptrs) {
	List ret(ptrs.size());
	for (int i = 0; i < ptrs.size(); i++) {
		SEXP s = ptrs[i];
		ret(i) = getPolygon((S2Polygon *) R_ExternalPtrAddr(s));
	}
	return ret;
}
