
#' Create an S2 LatLng Vector
#'
#' This class represents a latitude and longitude on the Earth's surface.
#' Most calculations in S2 convert this to a [as_s2_point()], which is a
#' unit vector representation of this value.
#'
#' @param lat,lng Vectors of latitude and longitude values in degrees.
#' @param x A [s2_latlng()] vector or an object that can be coerced to one.
#' @param ... Unused
#'
#' @return An object with class s2_latlng
#' @export
#'
#' @examples
#' s2_latlng(45, -64) # Halifax, Nova Scotia!
#' as.data.frame(s2_latlng(45, -64))
#'
s2_latlng <- function(lat, lng) {
  recycled <- recycle_common(as.double(lat), as.double(lng))
  new_s2_xptr(s2_latlng_from_numeric(recycled[[1]], recycled[[2]]), "s2_latlng")
}

#' @rdname s2_latlng
#' @export
as_s2_latlng <- function(x, ...) {
  UseMethod("as_s2_latlng")
}

#' @rdname s2_latlng
#' @export
as_s2_latlng.s2_latlng <- function(x, ...) {
  x
}

#' @rdname s2_latlng
#' @export
as_s2_latlng.s2_point <- function(x, ...) {
  new_s2_xptr(s2_latlng_from_s2_point(x), "s2_latlng")
}

#' @rdname s2_latlng
#' @export
as_s2_latlng.s2_geography <- function(x, ...) {
  new_s2_xptr(s2_latlng_from_numeric(cpp_s2_y(x), cpp_s2_x(x)), "s2_latlng")
}

#' @rdname s2_latlng
#' @export
as_s2_latlng.matrix <- function(x, ...) {
  s2_latlng(x[, 1, drop = TRUE], x[, 2, drop = TRUE])
}

#' @export
as_s2_latlng.character <- function(x, ...) {
  as_s2_latlng.wk_wkt(x)
}

#' @export
as_s2_latlng.wk_wkt <- function(x, ...) {
  meta <- wk::wkt_meta(x)
  if (any(meta$type_id != 1)) {
    stop("Can't import non-points as s2_latlng")
  }

  # need matching because empty points do not have a row in
  # wk::wk*_coords()
  coords <- wk::wkt_coords(x)
  coords_match <- match(seq_along(x), coords$feature_id)
  new_s2_xptr(s2_latlng_from_numeric(coords$y[coords_match], coords$x[coords_match]), "s2_latlng")
}

#' @export
as_s2_latlng.wk_wkb <- function(x, ...) {
  meta <- wk::wkb_meta(x)
  if (any(meta$type_id != 1)) {
    stop("Can't import non-points as s2_latlng")
  }

  # need matching because empty points do not have a row in
  # wk::wk*_coords()
  coords <- wk::wkb_coords(x)
  coords_match <- match(seq_along(x), coords$feature_id)
  new_s2_xptr(s2_latlng_from_numeric(coords$y[coords_match], coords$x[coords_match]), "s2_latlng")
}

#' @rdname s2_latlng
#' @export
as.data.frame.s2_latlng <- function(x, ...) {
  as.data.frame(data_frame_from_s2_latlng(x))
}

#' @rdname s2_latlng
#' @export
as.matrix.s2_latlng <- function(x, ...) {
  as.matrix(as.data.frame(data_frame_from_s2_latlng(x)))
}

#' @importFrom wk as_wkb
#' @export
as_wkb.s2_latlng <- function(x, ..., endian = wk::wk_platform_endian()) {
  df <- data_frame_from_s2_latlng(x)
  wk::new_wk_wkb(wk::coords_point_translate_wkb(df$lng, df$lat, endian = endian))
}

#' @importFrom wk as_wkt
#' @export
as_wkt.s2_latlng <- function(x, ...,  precision = 16, trim = TRUE) {
  df <- data_frame_from_s2_latlng(x)
  wk::new_wk_wkt(wk::coords_point_translate_wkt(df$lng, df$lat, precision = precision, trim = trim))
}

#' @export
`[<-.s2_latlng` <- function(x, i, value) {
  x <- unclass(x)
  x[i] <- as_s2_latlng(value)
  new_s2_xptr(x, "s2_latlng")
}

#' @export
`[[<-.s2_latlng` <- function(x, i, value) {
  x <- unclass(x)
  x[i] <- as_s2_latlng(value)
  new_s2_xptr(x, "s2_latlng")
}

#' @export
format.s2_latlng <- function(x, ...) {
  df <- as.data.frame(x)
  sprintf("(%s, %s)", format(df$lat, trim = TRUE), format(df$lng, trim = TRUE))
}
