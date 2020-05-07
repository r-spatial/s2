
#include <vector>
#include <memory>
#include <sstream>
#include "s2/s2latlng.h"
#include "s2/s2polyline.h"
#include "s2/s2polygon.h"
#include <Rcpp.h>
using namespace Rcpp;


// [[Rcpp::export]]
List s2polygon_from_s2polyline(List s2polyline, bool oriented, bool check) {

  SEXP item;
  std::vector<std::unique_ptr<S2Loop>> loops(s2polyline.size());

  for (R_xlen_t i = 0; i < s2polyline.size(); i++) {
    item = s2polyline[i];
    if (item == R_NilValue) {
      stop("Can't create s2polygon from missing s2polyline");
    } else {
      XPtr<S2Polyline> ptr(item);
      std::vector<S2Point> points(ptr->num_vertices());
      for (int j = 0; j < ptr->num_vertices(); j++) {
        points[j] = ptr->vertex(j).Normalize();
      }

      loops[i] = std::unique_ptr<S2Loop>(new S2Loop());
      loops[i]->set_s2debug_override(S2Debug::DISABLE);
      loops[i]->Init(points);

      // Not sure if && is short-circuiting in C++...
      if (check && !loops[i]->IsValid()) {
        S2Error error;
        loops[i]->FindValidationError(&error);
        stop(error.text());
      }
    }
  }

  XPtr<S2Polygon> polygon(new S2Polygon());
  if (oriented) {
    polygon->InitOriented(std::move(loops));
  } else {
    polygon->InitNested(std::move(loops));
  }

  return  List::create(polygon);
}

// [[Rcpp::export]]
List s2polyline_from_s2polygon(List s2polygon) {
  if (s2polygon.size() != 1) {
    stop("Can't convert an s2polygon of length != 1 to s2polyline");
  }

  SEXP item = s2polygon[0];
  if (item  == R_NilValue) {
    stop("Can't convert a missing s2polygon to s2polyline");
  }

  XPtr<S2Polygon> ptr(item);
  List output(ptr->num_loops());

  for (R_xlen_t i = 0; i < ptr->num_loops(); i++) {
    const S2Loop* loop = ptr->loop(i);
    std::vector<S2LatLng> vertices(loop->num_vertices());

    for (R_xlen_t j = 0; j < loop->num_vertices(); j++) {
      vertices[j] = S2LatLng(loop->vertex(j));
    }

    output[i] = XPtr<S2Polyline>(new S2Polyline(vertices));
  }

  return output;
}


// [[Rcpp::export]]
CharacterVector s2polygon_format(List s2polygon, int nVertices) {
  CharacterVector output(s2polygon.size());

  std::stringstream stream;
  SEXP item;
  S2LatLng vertex;

  for (R_xlen_t i = 0; i < s2polygon.size(); i++) {
    item = s2polygon[i];
    if (item == R_NilValue) {
      stream.str("NULL");
    } else {
      XPtr<S2Polygon> ptr(item);
      stream.str("");
      stream << "{" << ptr->num_loops() << "}";
      // if (ptr->num_loops() > 0) {
	  for (int l = 0, nVprinted = 0; nVprinted < nVertices && l < ptr->num_loops(); l++) {
        const S2Loop* loop = ptr->loop(l);
		stream << "[" <<  ptr->GetParent(l) << "]";

        for (int j = 0; nVprinted < nVertices && j < loop->num_vertices(); j++) {
          if (j > 0) {
            stream << " ";
          }
          vertex = S2LatLng(loop->vertex(j));
          stream << "(" << vertex.lat().degrees() << ", " << vertex.lng().degrees() << ")";
		  nVprinted++;
        }

      }
      if (nVertices < ptr->num_vertices()) {
        stream << "...+" << ptr->num_vertices() - nVertices;
      }
    }

    output[i] = stream.str();
  }
  return output;
}
