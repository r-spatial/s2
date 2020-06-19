
#' Geography Operation Options
#'
#' These functions specify defaults for options used to perform operations
#' and construct geometries.
#'
#' @param model,polygon_model,polyline_model See section 'Model'
#' @param snap Use `s2_snap_identity()`, `s2_snap_distance()`, `s2_snap_level()`,
#'   or `s2_snap_precision()` to specify how or if coordinate rounding should
#'   occur.
#' @param level A value from 0 to 30 corresponding to the cell level
#'   at which snapping should occur.
#' @param radius The radius to be used for specifying `distance`
#' @param distance A distance (in units of `radius`) denoting the maximum
#'   distance a vertex should move in the snapping process.
#' @param precision A number by which coordinates should be multiplied
#'   before being rounded. Rounded to the nearest exponent of 10.
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
#' and covers/covered_by (2) correspond to the SFA standard specification
#' of these operators.
#'
#' @export
#'
#' @examples
#' # use s2_options() to specify polygon/polyline models
#' # and/or snap level
#' s2_options(model = 1, snap = s2_snap_level(30))
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
s2_options <- function(model = -1, snap = s2_snap_identity(),
                       polygon_model = model, polyline_model = model) {
  structure(
    list(
      polygon_model = polygon_model,
      polyline_model = polyline_model,
      snap = snap
    ),
    class = "s2_options"
  )
}

#' @rdname s2_options
#' @export
s2_snap_identity <- function() {
  structure(list(), class = "snap_identity")
}

#' @rdname s2_options
#' @export
s2_snap_level <- function(level) {
  structure(list(level = level), class = "snap_level")
}

#' @rdname s2_options
#' @export
s2_snap_precision <- function(precision)  {
  structure(list(exponent = round(log10(precision))), class = "snap_precision")
}

#' @rdname s2_options
#' @export
s2_snap_distance <- function(distance, radius = s2_earth_radius_meters())  {
  structure(list(distance = distance / radius), class = "snap_distance")
}
