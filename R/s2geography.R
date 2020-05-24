
#' Create an S2 Geography Vector
#'
#' @param x An object that can be converted to an s2geography vector
#' @param oriented TRUE if polygon ring directions are known to be correct
#'   (i.e., exterior rings are defined counter clockwise and interior
#'   rings are defined clockwise).
#' @param ... Unused
#'
#' @return A [new_s2xptr()] with class s2geography
#' @export
#'
s2geography <- function(x, ...) {
  UseMethod("s2geography")
}

#' @rdname s2geography
#' @export
s2geography.s2geography <- function(x, ...) {
  x
}

#' @rdname s2geography
#' @export
s2geography.wk_wkb <- function(x, ..., oriented = FALSE) {
  new_s2xptr(s2geography_from_wkb(x, oriented = oriented), "s2geography")
}

#' @rdname s2geography
#' @export
s2geography.character <- function(x, ..., oriented = FALSE) {
  new_s2xptr(s2geography_from_wkt(x, oriented = oriented), "s2geography")
}

#' @export
format.s2geography <- function(x, ..., max_coords = 5) {
  paste0("<", s2geography_format(x, max_coords), ">")
}
