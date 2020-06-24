
#' Create an S2 Point Vector
#'
#' In S2 terminology, a "point" is a 3-dimensional unit vector representation
#' of an [s2_latlng()]. Internally, all s2 objects are stored as
#' 3-dimensional unit vectors.
#'
#' @param x,y,z Vectors of latitude and longitude values in degrees.
#' @param ... Unused
#'
#' @return An object with class s2_point
#' @export
#'
#' @examples
#' latlng <- s2_latlng(45, -64) # Halifax, Nova Scotia!
#' as_s2_point(latlng)
#' as.data.frame(as_s2_point(latlng))
#'
s2_point <- function(x, y, z) {
  recycled <- recycle_common(as.double(x), as.double(y), as.double(z))
  new_s2_xptr(s2_point_from_numeric(recycled[[1]], recycled[[2]], recycled[[3]]), "s2_point")
}

#' @rdname s2_point
#' @export
as_s2_point <- function(x, ...) {
  UseMethod("as_s2_point")
}

#' @rdname s2_point
#' @export
as_s2_point.s2_point <- function(x, ...) {
  x
}

#' @rdname s2_point
#' @export
as_s2_point.s2_latlng <- function(x, ...) {
  new_s2_xptr(s2_point_from_s2_latlng(x), "s2_point")
}

#' @rdname s2_point
#' @export
as_s2_point.s2_geography <- function(x, ...) {
  as_s2_point(as_s2_latlng(x))
}

#' @rdname s2_point
#' @export
as_s2_point.matrix <- function(x, ...) {
  s2_point(x[, 1, drop = TRUE], x[, 2, drop = TRUE], x[, 3, drop = TRUE])
}

#' @rdname s2_point
#' @export
as.data.frame.s2_point <- function(x, ...) {
  as.data.frame(data_frame_from_s2_point(x))
}

#' @rdname s2_point
#' @export
as.matrix.s2_point <- function(x, ...) {
  as.matrix(as.data.frame(data_frame_from_s2_point(x)))
}

#' @export
`[<-.s2_point` <- function(x, i, value) {
  x <- unclass(x)
  x[i] <- as_s2_point(value)
  new_s2_xptr(x, "s2_point")
}

#' @export
`[[<-.s2_point` <- function(x, i, value) {
  x <- unclass(x)
  x[i] <- as_s2_point(value)
  new_s2_xptr(x, "s2_point")
}

#' @export
format.s2_point <- function(x, ...) {
  df <- as.data.frame(x)
  sprintf(
    "[%s %s %s]",
    format(df$x, trim = TRUE),
    format(df$y, trim = TRUE),
    format(df$z, trim = TRUE)
  )
}
