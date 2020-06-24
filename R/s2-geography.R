
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
#' use these objects in [tibble::tibble()] and other packages that use the vctrs
#' framework.
#'
#' @param x An object that can be converted to an s2_geography vector
#' @param oriented TRUE if polygon ring directions are known to be correct
#'   (i.e., exterior rings are defined counter clockwise and interior
#'   rings are defined clockwise).
#' @param check Use `check = FALSE` to error on invalid geometries
#' @param endian The endian to use when writing well-known binary.
#'   Defaults to the platform endian. See [wk::as_wkb()].
#' @param precision The number of significant digits to export when
#'   writing well-known text. If `trim = FALSE`, the number of
#'   digits after the decimal place.
#' @param trim Should trailing zeroes be included after the decimal place?
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
as_s2_geography.default <- function(x, ...) {
  as_s2_geography(wk::as_wkb(x))
}

#' @rdname as_s2_geography
#' @export
as_s2_geography.s2_geography <- function(x, ...) {
  x
}

#' @rdname as_s2_geography
#' @export
as_s2_geography.s2_latlng <- function(x, ...) {
  df <- data_frame_from_s2_latlng(x)
  new_s2_xptr(cpp_s2_geog_point(df[[2]], df[[1]]), "s2_geography")
}

#' @rdname as_s2_geography
#' @export
as_s2_geography.s2_point <- function(x, ...) {
  as_s2_geography(as_s2_latlng(x))
}

#' @rdname as_s2_geography
#' @export
as_s2_geography.wk_wkb <- function(x, ..., oriented = FALSE, check = TRUE) {
  new_s2_xptr(
    s2_geography_from_wkb(x, oriented = oriented, check = check),
    "s2_geography"
  )
}

#' @rdname as_s2_geography
#' @export
as_s2_geography.WKB <- function(x, ..., oriented = FALSE, check = TRUE) {
  new_s2_xptr(
    s2_geography_from_wkb(x, oriented = oriented, check = check),
    "s2_geography"
  )
}

#' @rdname as_s2_geography
#' @export
as_s2_geography.blob <- function(x, ..., oriented = FALSE, check = TRUE) {
  new_s2_xptr(
    s2_geography_from_wkb(x, oriented = oriented, check = check),
    "s2_geography"
  )
}

#' @rdname as_s2_geography
#' @export
as_s2_geography.wk_wkt <- function(x, ..., oriented = FALSE, check = TRUE) {
  new_s2_xptr(
    s2_geography_from_wkt(x, oriented = oriented, check = check),
    "s2_geography"
  )
}

#' @rdname as_s2_geography
#' @export
as_s2_geography.character <- function(x, ..., oriented = FALSE, check = TRUE) {
  new_s2_xptr(
    s2_geography_from_wkt(x, oriented = oriented, check = check),
    "s2_geography"
  )
}

#' @rdname as_s2_geography
#' @export
as_s2_geography.logical <- function(x, ...) {
  stopifnot(isTRUE(x))
  new_s2_xptr(s2_geography_full(TRUE), "s2_geography")
}

#' @importFrom wk as_wkb
#' @rdname as_s2_geography
#' @export
as_wkb.s2_geography <- function(x, ..., endian = wk::wk_platform_endian()) {
  wk::new_wk_wkb(s2_geography_to_wkb(x, endian))
}

#' @importFrom wk as_wkt
#' @rdname as_s2_geography
#' @export
as_wkt.s2_geography <- function(x, ..., precision = 16, trim = TRUE) {
  wk::new_wk_wkt(s2_geography_to_wkt(x, precision = precision, trim = trim))
}


#' @export
`[<-.s2_geography` <- function(x, i, value) {
  x <- unclass(x)
  x[i] <- as_s2_geography(value)
  new_s2_xptr(x, "s2_geography")
}

#' @export
`[[<-.s2_geography` <- function(x, i, value) {
  x <- unclass(x)
  x[i] <- as_s2_geography(value)
  new_s2_xptr(x, "s2_geography")
}

#' @export
format.s2_geography <- function(x, ..., max_coords = 5) {
  paste0("<", s2_geography_format(x, max_coords), ">")
}

# this is what gets called by the RStudio viewer, for which
# format() is best suited (s2_as_text() is more explicit for WKT output)
#' @export
as.character.s2_geography <- function(x, ..., max_coords = 5) {
  format(x, ..., max_coords = max_coords)
}
