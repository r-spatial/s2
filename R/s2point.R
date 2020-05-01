
#' Create an s2point vector
#'
#' In S2 terminology, a "point" is a 3-dimensional unit vector, as opposed
#' to a [s2latlng()], which is a position on the Earth's surface.
#'
#' @param x,y,z Vectors of latitude and longitude values in degrees.
#' @param ... Unused
#'
#' @return A [new_s2xptr()] with class s2point
#' @export
#'
s2point <- function(x, ...) {
  UseMethod("s2point")
}

#' @rdname s2point
#' @export
s2point.s2point <- function(x, ...) {
  x
}

#' @rdname s2point
#' @export
s2point.numeric <- function(x, y, z, ...) {
  recycled <- recycle_common(x, y, z)
  new_s2xptr(s2point_from_numeric(recycled[[1]], recycled[[2]], recycled[[3]]), "s2point")
}

#' @rdname s2point
#' @export
s2point.matrix <- function(x, ...) {
  s2point.numeric(x[, 1, drop = TRUE], x[, 2, drop = TRUE], x[, 3, drop = TRUE])
}

#' @rdname s2point
#' @export
as.data.frame.s2point <- function(x, ...) {
  as.data.frame(data_frame_from_s2point(x))
}

#' @rdname s2point
#' @export
as.matrix.s2point <- function(x, ...) {
  as.matrix(as.data.frame(data_frame_from_s2point(x)))
}

#' @export
format.s2point <- function(x, ...) {
  df <- as.data.frame(x)
  sprintf(
    "[%s %s %s]",
    format(df$x, trim = TRUE),
    format(df$y, trim = TRUE),
    format(df$z, trim = TRUE)
  )
}
