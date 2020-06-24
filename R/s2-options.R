
#' Geography Operation Options
#'
#' These functions specify defaults for options used to perform operations
#' and construct geometries. These are used in predicates (e.g., [s2_intersects()]),
#' and boolean operations (e.g., [s2_intersection()]) to specify the model for
#' containment and how new geometries should be constructed.
#'
#' @param model,polygon_model,polyline_model See section 'Model'
#' @param snap Use `s2_snap_identity()`, `s2_snap_distance()`, `s2_snap_level()`,
#'   or `s2_snap_precision()` to specify how or if coordinate rounding should
#'   occur.
#' @param snap_radius As opposed to the snap function, which specifies
#'   the maximum distance a vertex should move, the snap radius (in radians) sets
#'   the minimum distance between vertices of the output that don't cause vertices
#'   to move more than the distance specified by the snap function. This can be used
#'   to simplify the result of a boolean operation.
#' @param level A value from 0 to 30 corresponding to the cell level
#'   at which snapping should occur.
#' @param distance A distance (in radians) denoting the maximum
#'   distance a vertex should move in the snapping process.
#' @param precision A number by which coordinates should be multiplied
#'   before being rounded. Rounded to the nearest exponent of 10.
#'
#' @section Model:
#' The geometry model indicates whether or not a geometry includes its boundaries.
#' Boundaries of line geometries are its end points.
#' OPEN geometries do not contain their boundary (`model = 0`); CLOSED
#' geometries (`model = 2`) contain their boundary; HALF-CLOSED geometries
#' (`model = 1`) contain half of their boundaries, such that when two polygons
#' do not overlap or two lines do not cross, no point exist that belong to
#' more than one of the geometries. (This latter form, half-closed, is
#' not present in the OpenGIS "simple feature access" (SFA) standard nor DE9-IM on
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
#' s2_contains("LINESTRING (0 0, 0 1, 1 1)", "POINT (0 0)", s2_options(model = 0))
#' s2_contains("LINESTRING (0 0, 0 1, 1 1)", "POINT (0 1)", s2_options(model = 0))
#' s2_contains("LINESTRING (0 0, 0 1, 1 1)", "POINT (0 0.5)", s2_options(model = 0))
#'
#' # in the semi-open and closed models, endpoints are contained
#' s2_contains("LINESTRING (0 0, 0 1, 1 1)", "POINT (0 0)", s2_options(model = 1))
#' s2_contains("LINESTRING (0 0, 0 1, 1 1)", "POINT (0 1)", s2_options(model = 1))
#' s2_contains("LINESTRING (0 0, 0 1, 1 1)", "POINT (0 0)", s2_options(model = 2))
#' s2_contains("LINESTRING (0 0, 0 1, 1 1)", "POINT (0 1)", s2_options(model = 2))
#'
#' # for polygons, boundary points are either contained or not contained depending on
#' # the model of  choice
#' s2_contains("POLYGON ((0 0, 0 1, 1 1, 0 0))", "POINT (0 0)", s2_options(model = 0))
#' s2_contains("POLYGON ((0 0, 0 1, 1 1, 0 0))", "POINT (0.5 0.75)", s2_options(model = 0))
#'
#' s2_contains("POLYGON ((0 0, 0 1, 1 1, 0 0))", "POINT (0 0)", s2_options(model = 1))
#' s2_contains("POLYGON ((0 0, 0 1, 1 1, 0 0))", "POINT (0.5 0.75)", s2_options(model = 1))
#' s2_contains("POLYGON ((0 0, 0 1, 1 1, 0 0))", "POINT (0 0)", s2_options(model = 2))
#' s2_contains("POLYGON ((0 0, 0 1, 1 1, 0 0))", "POINT (0.5 0.75)", s2_options(model = 2))
#'
#' # s2_dwithin(x, y, epsilon) is a more explicit test if boundaries are important
#' s2_dwithin(
#'   "LINESTRING (0 0, 0 1, 1 1)",
#'   c("POINT (0 0)", "POINT (0 1)", "POINT (0 0.5)"),
#'   1e-7
#' )
#'
s2_options <- function(model = -1, snap = s2_snap_identity(), snap_radius = -1,
                       polygon_model = model, polyline_model = model) {
  structure(
    list(
      polygon_model = polygon_model,
      polyline_model = polyline_model,
      snap = snap,
      snap_radius = snap_radius
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
s2_snap_distance <- function(distance)  {
  structure(list(distance = distance), class = "snap_distance")
}
