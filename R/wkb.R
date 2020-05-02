
#' Convert to and from well-known binary
#'
#' @inheritParams s2latlng
#' @inheritParams s2polygon
#' @param endian  The endian to use when writing well-known binary.
#'   Will eventually default to the platform endian.
#'
#' @return A well-known binary vector (`list()` of `raw()`)
#' @export
#'
as_wkb <- function(x, ...) {
  # this generic should live in the wk package, but that doesn't exist yet...
  UseMethod("as_wkb")
}

#' @rdname as_wkb
#' @export
as_wkb.s2latlng <- function(x, ..., endian = 1) {
  wkb <- wkb_from_s2latlng(x, endian)
  class(wkb) <- "wk_wkb"
  wkb
}

#' @rdname as_wkb
#' @export
as_wkb.s2polyline <- function(x, ..., endian = 1) {
  wkb <- wkb_from_s2polyline(x, endian)
  class(wkb) <- "wk_wkb"
  wkb
}

#' @rdname as_wkb
#' @export
as_wkb.s2polygon <- function(x, ..., endian = 1) {
  wkb <- wkb_from_s2polygon(x, endian)
  class(wkb) <- "wk_wkb"
  wkb
}

#' @rdname as_wkb
#' @export
s2latlng.wk_wkb <- function(lng, ...) {
  new_s2xptr(s2latlng_from_wkb(lng), "s2latlng")
}

#' @rdname as_wkb
#' @export
s2polyline.wk_wkb <- function(x, ...) {
  new_s2xptr(s2polyline_from_wkb(x), "s2polyline")
}

#' @rdname as_wkb
#' @export
s2polygon.wk_wkb <- function(x, ..., oriented = FALSE, check = TRUE) {
  new_s2xptr(s2polygon_from_wkb(x, oriented = oriented, check = check), "s2polygon")
}
