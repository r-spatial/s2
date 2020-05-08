
#' Create an s2polygon vector
#'
#' @inheritParams s2latlng
#' @param x An object from which an s2polygon can be created
#' @param oriented If `TRUE`, rings are guaranteed to be oriented (e.g. read
#'   by [sf::read_sf()] using `check_ring_dir = TRUE`, meaning CCW exterior rings
#'   and CW holes. If `FALSE`, rings are normalized and holes are deduced from
#'   degree of nesting.
#' @param check Pass `FALSE` to skip ring validation.
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
#' @export
s2polyline.s2polygon <- function(x, ..., close = FALSE) {
  new_s2xptr(s2polyline_from_s2polygon(x, close = close), "s2polyline")
}

#' @export
format.s2polygon <- function(x, ..., n_vertices = 5) {
  s2polygon_format(x, n_vertices)
}
