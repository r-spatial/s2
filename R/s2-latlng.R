
#' Create an s2_latlng vector
#'
#' This class represents a latitude and longitude on the Earth's surface.
#' Most calculations in S2 convert this to a [as_s2_point()], which is a
#' unit vector representation of this value.
#'
#' @param lat,lng Vectors of latitude and longitude values in degrees.
#' @param x A [as_s2_latlng()] vector
#' @param ... Unused
#'
#' @return A [new_s2_xptr()] with class s2_latlng
#' @export
#'
#' @examples
#' as_s2_latlng(45, -64) # Halifax, Nova Scotia!
#' as.data.frame(as_s2_latlng(45, -64))
#'
as_s2_latlng <- function(lat, ...) {
  UseMethod("as_s2_latlng")
}

#' @rdname as_s2_latlng
#' @export
as_s2_latlng.s2_latlng <- function(lat, ...) {
  lat
}

#' @rdname as_s2_latlng
#' @export
as_s2_latlng.s2_point <- function(lat, ...) {
  new_s2_xptr(s2_latlng_from_s2_point(lat), "s2_latlng")
}

#' @rdname as_s2_latlng
#' @export
as_s2_latlng.numeric <- function(lat, lng, ...) {
  recycled <- recycle_common(lat = lat, lng = lng)
  new_s2_xptr(s2_latlng_from_numeric(recycled$lat, recycled$lng), "s2_latlng")
}

#' @rdname as_s2_latlng
#' @export
as_s2_latlng.matrix <- function(lat, ...) {
  as_s2_latlng.numeric(lat[, 1, drop = TRUE], lat[, 2, drop = TRUE])
}

#' @export
as_s2_latlng.wk_wkb <- function(lat, ...) {
  new_s2_xptr(s2_latlng_from_wkb(lat), "s2_latlng")
}

#' @rdname as_s2_latlng
#' @export
as.data.frame.s2_latlng <- function(x, ...) {
  as.data.frame(data_frame_from_s2_latlng(x))
}

#' @rdname as_s2_latlng
#' @export
as.matrix.s2_latlng <- function(x, ...) {
  as.matrix(as.data.frame(data_frame_from_s2_latlng(x)))
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
