
#' Geography Operation Options
#'
#' These functions specify defaults for options used to perform operations
#' and construct geometries. These are used in predicates (e.g., [s2_intersects()]),
#' and boolean operations (e.g., [s2_intersection()]) to specify the model for
#' containment and how new geometries should be constructed.
#'
#' @param model One of 'open', 'semi-open' (default for polygons),
#'   or 'closed' (default for polylines). See section 'Model'
#' @param snap Use `s2_snap_identity()`, `s2_snap_distance()`, `s2_snap_level()`,
#'   or `s2_snap_precision()` to specify how or if coordinate rounding should
#'   occur.
#' @param snap_radius As opposed to the snap function, which specifies
#'   the maximum distance a vertex should move, the snap radius (in radians) sets
#'   the minimum distance between vertices of the output that don't cause vertices
#'   to move more than the distance specified by the snap function. This can be used
#'   to simplify the result of a boolean operation. Use -1 to specify that any
#'   minimum distance is acceptable.
#' @param duplicate_edges Use `TRUE` to keep duplicate edges (e.g., duplicate
#'   points).
#' @param edge_type One of 'directed' (default) or 'undirected'.
#' @param polyline_type One of 'path' (default) or 'walk'. If 'walk',
#'   polylines that backtrack are preserved.
#' @param polyline_sibling_pairs One of 'discard' (default) or 'keep'.
#' @param simplify_edge_chains Use `TRUE` to remove vertices that are within
#'   `snap_radius` of the original vertex.
#' @param split_crossing_edges Use `TRUE` to split crossing polyline edges
#'   when creating geometries.
#' @param idempotent Use `FALSE` to apply snap even if snapping is not necessary
#'   to satisfy vertex constraints.
#' @param validate Use `TRUE` to validate the result from the builder.
#' @param level A value from 0 to 30 corresponding to the cell level
#'   at which snapping should occur.
#' @param distance A distance (in radians) denoting the maximum
#'   distance a vertex should move in the snapping process.
#' @param precision A number by which coordinates should be multiplied
#'   before being rounded. Rounded to the nearest exponent of 10.
#' @param dimensions A combination of 'point', 'polyline', and/or 'polygon'
#'   that can used to constrain the output of [s2_rebuild()] or a
#'   boolean operation.
#'
#' @section Model:
#' The geometry model indicates whether or not a geometry includes its boundaries.
#' Boundaries of line geometries are its end points.
#' OPEN geometries do not contain their boundary (`model = "open"`); CLOSED
#' geometries (`model = "closed"`) contain their boundary; SEMI-OPEN geometries
#' (`model = "semi-open"`) contain half of their boundaries, such that when two polygons
#' do not overlap or two lines do not cross, no point exist that belong to
#' more than one of the geometries. (This latter form, half-closed, is
#' not present in the OpenGIS "simple feature access" (SFA) standard nor DE9-IM on
#' which that is based). The default values for [s2_contains()] (open)
#' and covers/covered_by (closed) correspond to the SFA standard specification
#' of these operators.
#'
#' @export
#'
#' @examples
#' # use s2_options() to specify containment models, snap level
#' # layer creation options, and builder options
#' s2_options(model = "closed", snap = s2_snap_level(30))
#'
s2_options <- function(model = NULL,
                       snap = s2_snap_identity(),
                       snap_radius = -1,
                       duplicate_edges = FALSE,
                       edge_type = "directed",
                       validate = FALSE,
                       polyline_type = "path",
                       polyline_sibling_pairs = "keep",
                       simplify_edge_chains = FALSE,
                       split_crossing_edges = FALSE,
                       idempotent = FALSE,
                       dimensions = c("point", "polyline", "polygon")) {
  # check snap radius (passing in a huge snap radius can cause problems)
  if (snap_radius > 3) {
    stop(
      "Snap radius is too large. Did you pass in a snap radius in meters instead of radians?",
      call. = FALSE
    )
  }

  structure(
    list(
      # model needs to be "unset" by default because there are differences in polygon
      # and polyline handling by default that are good defaults to preserve
      model = if (is.null(model)) -1 else match_option(model[1], c("open", "semi-open", "closed"), "model"),
      snap = snap,
      snap_radius = snap_radius,
      duplicate_edges = duplicate_edges,
      edge_type = match_option(edge_type[1], c("directed", "undirected"), "edge_type"),
      validate = validate,
      polyline_type = match_option(polyline_type[1], c("path", "walk"), "polyline_type"),
      polyline_sibling_pairs = match_option(
        polyline_sibling_pairs,
        c("discard", "keep"),
        "polyline_sibling_pairs"
      ),
      simplify_edge_chains = simplify_edge_chains,
      split_crossing_edges = split_crossing_edges,
      idempotent = idempotent,
      dimensions = match_option(dimensions, c("point", "polyline", "polygon"), "dimensions")
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
  if (level > 30) {
    stop("`level` must be an intger between 1 and 30", call. = FALSE)
  }

  structure(list(level = level), class = "snap_level")
}

#' @rdname s2_options
#' @export
s2_snap_precision <- function(precision)  {
  stopifnot(precision <= 1e17) # https://github.com/r-spatial/s2/issues/248
  structure(list(exponent = round(log10(precision))), class = "snap_precision")
}

#' @rdname s2_options
#' @export
s2_snap_distance <- function(distance)  {
  structure(list(distance = distance), class = "snap_distance")
}


match_option <- function(x, options, arg) {
  result <- match(x, options)
  if (any(is.na(result))) {
    stop(
      sprintf("`%s` must be one of %s", arg, paste0('"', options, '"', collapse = ", ")),
      call. = FALSE
    )
  }

  result
}
