
#' Create an S2 Geography Vector
#'
#' Geography vectors are arrays of points, lines, polygons, and/or collections
#' of these. Geography vectors assume coordinates are longitude and latitude
#' on a perfect sphere.
#'
#' The coercion function [as_s2_geography()] is used to wrap the input
#' of most functions in the s2 package so that you can use other objects with
#' an unambiguious interpretation as a geography vector. Geography vectors
#' have a minimal [vctrs][vctrs::vctrs-package] implementation, so you can
#' use these objects in tibble, dplyr, and other packages that use the vctrs
#' framework.
#'
#' @param x An object that can be converted to an s2_geography vector
#' @param oriented TRUE if polygon ring directions are known to be correct
#'   (i.e., exterior rings are defined counter clockwise and interior
#'   rings are defined clockwise).
#' @param check Use `check = FALSE` to skip error on invalid geometries
#' @param ... Unused
#'
#' @return An object with class s2_geography
#' @export
#'
#' @seealso
#' [s2_geog_from_wkb()], [s2_geog_from_text()], [s2_geog_point()],
#' [s2_make_line()], [s2_make_polygon()] for other ways to
#' create geography vectors, and [s2_as_binary()] and [s2_as_text()]
#' for other ways to export them.
#'
as_s2_geography <- function(x, ...) {
  UseMethod("as_s2_geography")
}

#' @rdname as_s2_geography
#' @export
s2_geography <- function() {
  new_s2_geography(list())
}

#' @rdname as_s2_geography
#' @export
as_s2_geography.s2_geography <- function(x, ...) {
  x
}

#' @rdname as_s2_geography
#' @export
as_s2_geography.wk_xy <- function(x, ...) {
  x <- as_s2_lnglat(x)
  df <- unclass(x)
  s2_geog_point(df[[1]], df[[2]])
}

#' @rdname as_s2_geography
#' @export
as_s2_geography.wk_wkb <- function(x, ..., oriented = FALSE, check = TRUE) {
  if (identical(wk::wk_is_geodesic(x), FALSE)) {

    # points and an empty vector are OK and shouldn't trigger an error
    meta <- wk::wk_meta(x)
    if (!all(meta$geometry_type %in% c(1, 4, NA), na.rm = TRUE)) {
      stop(
        paste0(
          "Can't create s2_geography from Cartesian wkb().\n",
          "Use `wk_set_geodesic(x, TRUE)` to assert that edges can be\n",
          "interpolated along the sphere."
        ),
        call. = FALSE
      )
    }
  }

  wk::wk_handle(
    x,
    s2_geography_writer(oriented = oriented, check = check)
  )
}

#' @rdname as_s2_geography
#' @export
as_s2_geography.WKB <- function(x, ..., oriented = FALSE, check = TRUE) {
  s2_geog_from_wkb(x, oriented = oriented, check = check)
}

#' @rdname as_s2_geography
#' @export
as_s2_geography.blob <- function(x, ..., oriented = FALSE, check = TRUE) {
  s2_geog_from_wkb(x, oriented = oriented, check = check)
}

#' @rdname as_s2_geography
#' @export
as_s2_geography.wk_wkt <- function(x, ..., oriented = FALSE, check = TRUE) {
  if (identical(wk::wk_is_geodesic(x), FALSE)) {

    # points and an empty vector are OK and shouldn't trigger an error
    meta <- wk::wk_meta(x)
    if (!all(meta$geometry_type %in% c(1, 4, NA), na.rm = TRUE)) {
      stop(
        paste0(
          "Can't create s2_geography from Cartesian wkt().\n",
          "Use `wk_set_geodesic(x, TRUE)` to assert that edges can be\n",
          "interpolated along the sphere."
        ),
        call. = FALSE
      )
    }
  }

  wk::wk_handle(
    x,
    s2_geography_writer(oriented = oriented, check = check)
  )
}

#' @rdname as_s2_geography
#' @export
as_s2_geography.character <- function(x, ..., oriented = FALSE, check = TRUE) {
  s2_geog_from_text(x, oriented = oriented, check = check)
}

#' @rdname as_s2_geography
#' @export
as_s2_geography.logical <- function(x, ...) {
  stopifnot(isTRUE(x))
  new_s2_geography(s2_geography_full(TRUE))
}

#' @importFrom wk as_wkb
#' @rdname as_s2_geography
#' @export
as_wkb.s2_geography <- function(x, ...) {
  wkb <- wk::wk_handle(x, wk::wkb_writer())
  wk::wk_is_geodesic(wkb) <- TRUE
  wk::wk_crs(wkb) <- wk::wk_crs_longlat()
  wkb
}

#' @importFrom wk as_wkt
#' @rdname as_s2_geography
#' @export
as_wkt.s2_geography <- function(x, ...) {
  wkt <- wk::wk_handle(x, wk::wkt_writer())
  wk::wk_is_geodesic(wkt) <- TRUE
  wk::wk_crs(wkt) <- wk::wk_crs_longlat()
  wkt
}

#' @importFrom wk wk_crs
#' @export
wk_crs.s2_geography <- function(x) {
  wk::wk_crs_longlat()
}

#' @importFrom wk wk_set_crs
#' @export
wk_set_crs.s2_geography <- function(x, crs) {
  if (!wk::wk_crs_equal(crs, wk::wk_crs(x))) {
    warning("Setting the crs of s2_geography() is not supported")
  }

  x
}

#' @importFrom wk wk_is_geodesic
#' @export
wk_is_geodesic.s2_geography <- function(x) {
  TRUE
}

#' @importFrom wk wk_set_geodesic
#' @export
wk_set_geodesic.s2_geography <- function(x, geodesic) {
  if (!isTRUE(geodesic)) {
    stop("Can't set geodesic of s2_geography to FALSE")
  }

  x
}

new_s2_geography <- function(x) {
  structure(x, class = c("s2_geography", "wk_vctr"))
}

#' @export
is.na.s2_geography <- function(x) {
  cpp_s2_geography_is_na(x)
}

#' @export
`[<-.s2_geography` <- function(x, i, value) {
  x <- unclass(x)
  x[i] <- as_s2_geography(value)
  new_s2_geography(x)
}

#' @export
`[[<-.s2_geography` <- function(x, i, value) {
  x <- unclass(x)
  x[i] <- as_s2_geography(value)
  new_s2_geography(x)
}

#' @export
format.s2_geography <- function(x, ..., max_coords = 5, precision = 9, trim = TRUE) {
  wk::wk_format(x, precision = precision, max_coords = max_coords, trim = trim)
}

# this is what gets called by the RStudio viewer, for which
# format() is best suited (s2_as_text() is more explicit for WKT output)
#' @export
as.character.s2_geography <- function(x, ..., max_coords = 5, precision = 9, trim = TRUE) {
  format(x, ..., max_coords = max_coords, precision = precision, trim = trim)
}
