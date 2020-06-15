
#' Create an S2 Geography Vector
#'
#' @param x An object that can be converted to an s2_geography vector
#' @param oriented TRUE if polygon ring directions are known to be correct
#'   (i.e., exterior rings are defined counter clockwise and interior
#'   rings are defined clockwise).
#' @param ... Unused
#'
#' @return An object with class s2_geography
#' @export
#'
as_s2_geography <- function(x, ...) {
  UseMethod("as_s2_geography")
}

#' @rdname as_s2_geography
#' @export
as_s2_geography.s2_geography <- function(x, ...) {
  x
}

#' @rdname as_s2_geography
#' @export
as_s2_geography.wk_wkb <- function(x, ..., oriented = FALSE) {
  new_s2_xptr(s2_geography_from_wkb(x, oriented = oriented), "s2_geography")
}

#' @rdname as_s2_geography
#' @export
as_s2_geography.logical <- function(x, ...) {
  stopifnot(isTRUE(x))
  new_s2_xptr(s2_geography_full(TRUE), "s2_geography")
}


#' @rdname as_s2_geography
#' @export
as_s2_geography.character <- function(x, ..., oriented = FALSE) {
  new_s2_xptr(s2_geography_from_wkt(x, oriented = oriented), "s2_geography")
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
