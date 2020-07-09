
#' Create an S2 LngLat Vector
#'
#' This class represents a latitude and longitude on the Earth's surface.
#' Most calculations in S2 convert this to a [as_s2_point()], which is a
#' unit vector representation of this value.
#'
#' @param lat,lng Vectors of latitude and longitude values in degrees.
#' @param x A [s2_lnglat()] vector or an object that can be coerced to one.
#' @param ... Unused
#'
#' @return An object with class s2_lnglat
#' @export
#'
#' @examples
#' s2_lnglat(45, -64) # Halifax, Nova Scotia!
#' as.data.frame(s2_lnglat(45, -64))
#'
s2_lnglat <- function(lng, lat) {
  recycled <- recycle_common(as.double(lng), as.double(lat))
  new_s2_xptr(s2_lnglat_from_numeric(recycled[[1]], recycled[[2]]), "s2_lnglat")
}

#' @rdname s2_lnglat
#' @export
as_s2_lnglat <- function(x, ...) {
  UseMethod("as_s2_lnglat")
}

#' @rdname s2_lnglat
#' @export
as_s2_lnglat.s2_lnglat <- function(x, ...) {
  x
}

#' @rdname s2_lnglat
#' @export
as_s2_lnglat.s2_point <- function(x, ...) {
  new_s2_xptr(s2_lnglat_from_s2_point(x), "s2_lnglat")
}

#' @rdname s2_lnglat
#' @export
as_s2_lnglat.s2_geography <- function(x, ...) {
  new_s2_xptr(s2_lnglat_from_numeric(cpp_s2_x(x), cpp_s2_y(x)), "s2_lnglat")
}

#' @rdname s2_lnglat
#' @export
as_s2_lnglat.matrix <- function(x, ...) {
  s2_lnglat(x[, 1, drop = TRUE], x[, 2, drop = TRUE])
}

#' @export
as_s2_lnglat.character <- function(x, ...) {
  as_s2_lnglat.wk_wkt(x)
}

#' @export
as_s2_lnglat.wk_wkt <- function(x, ...) {
  as_s2_lnglat(as_s2_geography(x), ...)
}

#' @export
as_s2_lnglat.wk_wkb <- function(x, ...) {
  as_s2_lnglat(as_s2_geography(x), ...)
}

#' @rdname s2_lnglat
#' @export
as.data.frame.s2_lnglat <- function(x, ...) {
  as.data.frame(data_frame_from_s2_lnglat(x))
}

#' @rdname s2_lnglat
#' @export
as.matrix.s2_lnglat <- function(x, ...) {
  as.matrix(as.data.frame(data_frame_from_s2_lnglat(x)))
}

#' @importFrom wk as_wkb
#' @export
as_wkb.s2_lnglat <- function(x, ..., endian = wk::wk_platform_endian()) {
  as_wkb(as_s2_geography(x), ..., endian = endian)
}

#' @importFrom wk as_wkt
#' @export
as_wkt.s2_lnglat <- function(x, ...,  precision = 16, trim = TRUE) {
  as_wkt(as_s2_geography(x), ..., precision = precision, trim = trim)
}

#' @export
`[<-.s2_lnglat` <- function(x, i, value) {
  x <- unclass(x)
  x[i] <- as_s2_lnglat(value)
  new_s2_xptr(x, "s2_lnglat")
}

#' @export
`[[<-.s2_lnglat` <- function(x, i, value) {
  x <- unclass(x)
  x[i] <- as_s2_lnglat(value)
  new_s2_xptr(x, "s2_lnglat")
}

#' @export
format.s2_lnglat <- function(x, ...) {
  df <- as.data.frame(x)
  sprintf("(%s, %s)", format(df$lng, trim = TRUE), format(df$lat, trim = TRUE))
}
