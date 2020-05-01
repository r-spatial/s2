
#include <vector>
#include <sstream>
#include "s2/s2latlng.h"
#include "s2/s2polyline.h"

#include <Rcpp.h>
using namespace Rcpp;

// [[Rcpp::export]]
List s2polyline_from_s2latlng(List s2latlng) {
  std::vector<S2LatLng> vertices(s2latlng.size());
  SEXP item;
  for (R_xlen_t i = 0; i < s2latlng.size(); i++) {
    item = s2latlng[i];
    if (item == R_NilValue) {
      stop("Can't create s2polyline from missing coordinates");
    } else {
      XPtr<S2LatLng> ptr(item);
      vertices[i] = S2LatLng(*ptr);
    }
  }

  XPtr<S2Polyline> polylinePtr(new S2Polyline());
  polylinePtr->Init(vertices);
  return List::create(polylinePtr);
}

// [[Rcpp::export]]
List s2polyline_to_s2latlng(List s2polyline) {
  if (s2polyline.size() != 1) {
    stop("Can't convert an s2polyline of length != 1 to s2latlng");
  }

  SEXP item = s2polyline[0];
  if (item  == R_NilValue) {
    stop("Can't convert a missing s2polyline to s2latlng");
  }

  XPtr<S2Polyline> ptr(item);
  List output(ptr->num_vertices());
  for (R_xlen_t i = 0; i < ptr->num_vertices(); i++) {
    output[i] = XPtr<S2LatLng>(new S2LatLng(ptr->vertex(i)));
  }

  return output;
}

// [[Rcpp::export]]
CharacterVector s2polyline_format(List s2polyline, int nVertices) {
  CharacterVector output(s2polyline.size());

  std::stringstream stream;
  SEXP item;
  S2LatLng vertex;

  for (R_xlen_t i = 0; i < s2polyline.size(); i++) {
    item = s2polyline[i];
    if (item == R_NilValue) {
      stream.str("NULL");
    } else {
      XPtr<S2Polyline> ptr(item);
      stream.str("");
      stream << "/ ";

      for (int j = 0; j < std::min(nVertices, ptr->num_vertices()); j++) {
        if (j > 0) {
          stream << " ";
        }
        vertex = S2LatLng(ptr->vertex(j));
        stream << "(" << vertex.lat().degrees() << ", " << vertex.lng().degrees() << ")";
      }

      if (nVertices < ptr->num_vertices()) {
        stream << "...+" << ptr->num_vertices() - nVertices;
      }
    }

    output[i] = stream.str();
  }
  return output;
}
