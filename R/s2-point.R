
#' Create an S2 Point Vector
#'
#' In S2 terminology, a "point" is a 3-dimensional unit vector representation
#' of an [s2_point()]. Internally, all s2 objects are stored as
#' 3-dimensional unit vectors.
#'
#' @param x,y,z Vectors of latitude and longitude values in degrees.
#' @param ... Unused
#'
#' @return An object with class s2_point
#' @export
#'
#' @examples
#' point <- s2_lnglat(-64, 45) # Halifax, Nova Scotia!
#' as_s2_point(point)
#' as.data.frame(as_s2_point(point))
#'
s2_point <- function(x, y, z) {
  wk::xyz(x, y, z, crs = s2_point_crs())
}

#' @rdname s2_point
#' @export
s2_point_crs <- function() {
  structure(list(), class = "s2_point_crs")
}

#' @export
format.s2_point_crs <- function(x, ...) {
  "s2_point_crs"
}

#' @rdname s2_point
#' @export
as_s2_point <- function(x, ...) {
  UseMethod("as_s2_point")
}

#' @rdname s2_point
#' @export
as_s2_point.default <- function(x, ...) {
  as_s2_point(wk::as_xy(x))
}

#' @rdname s2_point
#' @export
as_s2_point.wk_xy <- function(x, ...) {
  stopifnot(wk::wk_crs_equal(wk::wk_crs(x), wk::wk_crs_longlat()))
  wk::new_wk_xyz(
    s2_point_from_s2_lnglat(x),
    crs = s2_point_crs()
  )
}

#' @rdname s2_point
#' @export
as_s2_point.wk_xyz <- function(x, ...) {
  wk::wk_set_crs(
    wk::as_xy(x, dims = c("x", "y", "z")),
    s2_point_crs()
  )
}

#' @export
as_s2_point.character <- function(x, ...) {
  as_s2_point(wk::new_wk_wkt(x))
}
