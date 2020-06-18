
#' Geography Operation Options
#'
#' These functions specify defaults for options used to perform operations
#' and construct geometries.
#'
#' @section Model:
#' The geometry model indicates whether a geometry includes its boundaries.
#' Boundaries of line geometries are its end points.
#' OPEN geometries do not contain their boundary (model = 0); CLOSED
#' geometries (model = 2) contain their boundary; HALF-CLOSED geometries
#' contain, like, half of their boundaries, such that when two polygons
#' do not overlap or two lines do not cross, no point exist that belong to
#' more than one of the geometries. (This latter form, half-closed, is
#' not present n the OpenGIS "simple feature access" (SFA) standard, or DE9-IM on
#' which that is based). A value of -1 does not set the model, leaving the
#' S2 default (HALF-CLOSED). The default values for [s2_contains()] (0)
#' and covers/coveredby (2) correspond to the SFA standard specification
#' of these operators.
#'
#' @export
#'
#' @examples
#' s2_snap_default()
#' s2_model_default()
#'
s2_snap_default <- function() {
  -1
}

#' @rdname s2_snap_default
#' @export
s2_model_default <- function() {
  -1
}
