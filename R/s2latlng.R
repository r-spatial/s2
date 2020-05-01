
#' Create an s2latlng vector
#'
#' @param lat,lng Vectors of latitude and longitude values in degrees.
#' @param ... Unused
#'
#' @return A [new_s2xptr()] with class s2latlng
#' @export
#'
s2latlng <- function(lat, ...) {
  UseMethod("s2latlng")
}

#' @rdname s2latlng
#' @export
s2latlng.s2latlng <- function(lat, ...) {
  lat
}

#' @rdname s2latlng
#' @export
s2latlng.numeric <- function(lat, lng, ...) {
  recycled <- recycle_common(lat = lat, lng = lng)
  new_s2xptr(s2latlng_from_numeric(recycled$lat, recycled$lng), "s2latlng")
}

#' @rdname s2latlng
#' @export
s2latlng.matrix <- function(lat, ...) {
  s2latlng.numeric(lat[, 1, drop = TRUE], lat[, 2, drop = TRUE])
}

#' @rdname s2latlng
#' @export
as.data.frame.s2latlng <- function(x, ...) {
  as.data.frame(data_frame_from_s2latlng(x))
}

#' @export
format.s2latlng <- function(x, ...) {
  df <- as.data.frame(x)
  sprintf("(%s, %s)", format(df$lat, trim = TRUE), format(df$lng, trim = TRUE))
}
