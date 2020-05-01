
#' Create an s2polygon vector
#'
#' @inheritParams s2latlng
#' @param x An object from which an s2polygon can be created
#'
#' @return A [new_s2xptr()] with class s2polygon
#' @export
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
s2polyline.s2polygon <- function(x, ...) {
  new_s2xptr(s2polyline_from_s2polygon(x), "s2polyline")
}

#' @export
format.s2polygon <- function(x, ..., n_vertices = 5) {
  s2polygon_format(x, n_vertices)
}
