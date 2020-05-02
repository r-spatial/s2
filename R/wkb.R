
#' @rdname s2latlng
#' @export
s2latlng.wk_wkb <- function(lat, ...) {
  new_s2xptr(s2latlng_from_wkb(lat), "s2latlng")
}

#' @rdname s2polyline
#' @export
s2polyline.wk_wkb <- function(x, ...) {
  new_s2xptr(s2polyline_from_wkb(x), "s2polyline")
}

#' @rdname s2polygon
#' @export
s2polygon.wk_wkb <- function(x, ...) {
  new_s2xptr(s2polygon_from_wkb(x), "s2polygon")
}
