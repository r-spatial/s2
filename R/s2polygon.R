
#' Create an s2polygon vector
#'
#' @inheritParams s2latlng
#' @param x An object from which an s2polygon can be created
#' @param oriented If `TRUE`, rings are guaranteed to be oriented (e.g. read
#'   by `sf::read_sf()` using `check_ring_dir = TRUE`, meaning CCW exterior rings
#'   and CW holes. If `FALSE`, rings are normalized and holes are deduced from
#'   degree of nesting. (Note that package sf checks ring directions in R2, not S2, 
#'   and may be useless when rings cross the antimeridian or cover a pole.)
#' @param check logical; pass `FALSE` to skip ring and polygon validity checks.
#'
#' @return A [new_s2xptr()] with class s2polygon
#' @export
#'
#' @examples
#' # construct from a polyline
#' latlng_loop <- s2latlng(c(0, 10, 0), c(0, 0, 10))
#' (polygon <- s2polygon(s2polyline(latlng_loop)))
#'
#' # convert back to polyline and latlng
#' s2polyline(polygon)
#' s2latlng(s2polyline(polygon))
#'
s2polygon <- function(x, ...) {
  UseMethod("s2polygon")
}

#' @rdname s2polygon
#' @export
s2polygon.s2polygon <- function(x, ...) {
  x
}

#' @rdname s2polygon
#' @export
s2polygon.s2polyline <- function(x, ...,  oriented = FALSE, check = TRUE) {
  new_s2xptr(s2polygon_from_s2polyline(x, oriented, check), "s2polygon")
}

#' @rdname s2polygon
#' @param close logical; if TRUE, the start point of the polygon is added as final point (adding the last polygon edge)
#' @export
s2polyline.s2polygon <- function(x, ..., close = FALSE) {
  new_s2xptr(s2polyline_from_s2polygon(x, close = close), "s2polyline")
}

#' @export
format.s2polygon <- function(x, ..., n_vertices = 5) {
  s2polygon_format(x, n_vertices)
}
