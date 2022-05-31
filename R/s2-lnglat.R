
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
  wk::xy(lng, lat, crs = wk::wk_crs_longlat())
}

#' @rdname s2_lnglat
#' @export
as_s2_lnglat <- function(x, ...) {
  UseMethod("as_s2_lnglat")
}

#' @rdname s2_lnglat
#' @export
as_s2_lnglat.default <- function(x, ...) {
  as_s2_lnglat(wk::as_xy(x))
}

#' @rdname s2_lnglat
#' @export
as_s2_lnglat.wk_xy <- function(x, ...) {
  wk::wk_set_crs(
    wk::as_xy(x, dims = c("x", "y")),
    wk::wk_crs_longlat()
  )
}

#' @rdname s2_lnglat
#' @export
as_s2_lnglat.wk_xyz <- function(x, ...) {
  if (wk::wk_crs_equal(wk::wk_crs(x), s2_point_crs())) {
    wk::new_wk_xy(
      s2_lnglat_from_s2_point(x),
      crs =  wk::wk_crs_longlat()
    )
  } else {
    NextMethod()
  }
}

#' @export
as_s2_lnglat.character <- function(x, ...) {
  as_s2_lnglat(wk::new_wk_wkt(x))
}
