
#' Create a s2polyline vector
#'
#' S2 polylines represent a collection of vertices composed of
#' [s2latlng()] objects.
#'
#' @param x An object from which an s2polyline can be created
#'
#' @return A [new_s2xptr()] with class s2polyline
#'
#' @examples
#' # construct and export a polyline using s2latlng()
#' (polyline <- s2polyline(s2latlng(45:50, -64)))
#' s2latlng(polyline)
#'
s2polyline <- function(x, ...) {
  UseMethod("s2polyline")
}

#' @rdname s2polyline
#' @export
s2polyline.s2polyline <- function(x, ...) {
  x
}

#' @rdname s2polyline
#' @export
s2polyline.s2latlng <- function(x, ...) {
  new_s2xptr(s2polyline_from_s2latlng(x), "s2polyline")
}

#' @rdname s2polyline
#' @export
s2latlng.s2polyline <- function(x, ...) {
  new_s2xptr(s2polyline_to_s2latlng(x), "s2latlng")
}

#' @export
format.s2polyline <- function(x, ..., n_vertices = 5) {
  s2polyline_format(x, nVertices = n_vertices)
}
