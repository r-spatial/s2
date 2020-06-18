
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
#' # get default values for snap_level and model
#' s2_snap_default()
#' s2_model_default()
#'
#' # model value affects boolean operations and binary predicates
#' # in the open model, lines do not contain endpoints (but do contain other points)
#' s2_contains("LINESTRING (0 0, 0 1, 1 1)", "POINT (0 0)", model = 0)
#' s2_contains("LINESTRING (0 0, 0 1, 1 1)", "POINT (0 1)", model = 0)
#' s2_contains("LINESTRING (0 0, 0 1, 1 1)", "POINT (0 0.5)", model = 0)
#'
#' # in the semi-open and closed models, endpoints are contained
#' s2_contains("LINESTRING (0 0, 0 1, 1 1)", "POINT (0 0)", model = 1)
#' s2_contains("LINESTRING (0 0, 0 1, 1 1)", "POINT (0 1)", model = 1)
#' s2_contains("LINESTRING (0 0, 0 1, 1 1)", "POINT (0 0.5)", model = 1)
#'
#' s2_contains("LINESTRING (0 0, 0 1, 1 1)", "POINT (0 0)", model = 2)
#' s2_contains("LINESTRING (0 0, 0 1, 1 1)", "POINT (0 1)", model = 2)
#' s2_contains("LINESTRING (0 0, 0 1, 1 1)", "POINT (0 0.5)", model = 2)
#'
#' # for polygons, the still does not include the edges between boundaries
#' s2_contains("POLYGON ((0 0, 0 1, 1 1, 0 0))", "POINT (0 0)", model = 0)
#' s2_contains("POLYGON ((0 0, 0 1, 1 1, 0 0))", "POINT (0.5 0.75)", model = 0)
#' s2_contains("POLYGON ((0 0, 0 1, 1 1, 0 0))", "POINT (0 0.5)", model = 0)
#'
#' s2_contains("POLYGON ((0 0, 0 1, 1 1, 0 0))", "POINT (0 0)", model = 1)
#' s2_contains("POLYGON ((0 0, 0 1, 1 1, 0 0))", "POINT (0.5 0.75)", model = 1)
#' s2_contains("POLYGON ((0 0, 0 1, 1 1, 0 0))", "POINT (0 0.5)", model = 1)
#'
#' s2_contains("POLYGON ((0 0, 0 1, 1 1, 0 0))", "POINT (0 0)", model = 2)
#' s2_contains("POLYGON ((0 0, 0 1, 1 1, 0 0))", "POINT (0.5 0.75)", model = 2)
#' s2_contains("POLYGON ((0 0, 0 1, 1 1, 0 0))", "POINT (0 0.5)", model = 2)
#'
#' # s2_dwithin(x, y, epsilon) is a more reliable test if boundaries are important
#' s2_dwithin(
#'   "LINESTRING (0 0, 0 1, 1 1)",
#'   c("POINT (0 0)", "POINT (0 1)", "POINT (0 0.5)"),
#'   1e-7
#' )
#'
s2_snap_default <- function() {
  -1
}

#' @rdname s2_snap_default
#' @export
s2_model_default <- function() {
  -1
}
