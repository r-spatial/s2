#include "s2/s2builder.h"
#include "s2/s2builderutil_s2polyline_layer.h"
#include "s2/s2builderutil_s2polygon_layer.h"
#include "s2/s2error.h"
#include <Rcpp.h>

using namespace Rcpp;
using namespace s2builderutil;

//' Build s2Polygons Layers
//' 
//' @param ptrs list with S2Polygons pointers
//' @param b_ptr external pointer with existing build, or NULL
//' @export
//[[Rcpp::export]]
List s2BuildPolygonsLayer(List ptrs, SEXP b_ptr) {
	std::vector<S2Polygon *> p(ptrs.size());
	for (int i = 0; i < ptrs.size(); i++) {
		SEXP s = ptrs[i];
		p[i] = (S2Polygon *) R_ExternalPtrAddr(s);
	}

	// S2Polygon *polygon = new S2Polygon(std::move(loops), S2Debug::DISABLE);
	// S2Polygon::Shape shape = polygon->Shape; //???
	S2Builder *b;
	if (b_ptr == R_NilValue)
		b = new S2Builder;
	else
		b = (S2Builder *) R_ExternalPtrAddr(b_ptr);
	auto p1 = new S2Polygon();
	auto pl = S2PolygonLayer(p1);
	// builder.StartLayer(S2PolylineLayer(line1));
	b->StartLayer(absl::make_unique<S2PolygonLayer>(pl));
// Add edges using builder.addEdge(), etc ...
	// builder.StartLayer(new S2PolylineLayer(line2));
// Add edges using builder.addEdge(), etc ...
	S2Error error;
	bool ret = b->Build(&error); // , error.toString());  // Builds "line1" & "line2"
	// builder.StartLayer(std::unique_ptr<S2Polygon>(polygon));
	// builder.AddPolygon(polygon);
	List lst(2);
	lst[0] = R_MakeExternalPtr((void *) b, R_NilValue, R_NilValue);
	lst[1] = ret;
	return lst;
}
