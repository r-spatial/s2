
#' Create an s2_latlng vector
#'
#' This class represents a latitude and longitude on the Earth's surface.
#' Most calculations in S2 convert this to a [s2point()], which is a
#' unit vector representation of this value.
#'
#' @param lat,lng Vectors of latitude and longitude values in degrees.
#' @param x A [s2_latlng()] vector
#' @param ... Unused
#'
#' @return A [new_s2xptr()] with class s2_latlng
#' @export
#'
#' @examples
#' s2_latlng(45, -64) # Halifax, Nova Scotia!
#' as.data.frame(s2_latlng(45, -64))
#'
s2_latlng <- function(lat, ...) {
  UseMethod("s2_latlng")
}

#' @rdname s2_latlng
#' @export
s2_latlng.s2_latlng <- function(lat, ...) {
  lat
}

#' @rdname s2_latlng
#' @export
s2_latlng.s2point <- function(lat, ...) {
  new_s2xptr(s2_latlng_from_s2point(lat), "s2_latlng")
}

#' @rdname s2_latlng
#' @export
s2_latlng.numeric <- function(lat, lng, ...) {
  recycled <- recycle_common(lat = lat, lng = lng)
  new_s2xptr(s2_latlng_from_numeric(recycled$lat, recycled$lng), "s2_latlng")
}

#' @rdname s2_latlng
#' @export
s2_latlng.matrix <- function(lat, ...) {
  s2_latlng.numeric(lat[, 1, drop = TRUE], lat[, 2, drop = TRUE])
}

#' @export
s2_latlng.wk_wkb <- function(lat, ...) {
  new_s2xptr(s2_latlng_from_wkb(lat), "s2_latlng")
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

#' @export
format.s2_latlng <- function(x, ...) {
  df <- as.data.frame(x)
  sprintf("(%s, %s)", format(df$lat, trim = TRUE), format(df$lng, trim = TRUE))
}
