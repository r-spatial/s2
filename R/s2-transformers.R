
#' S2 Geography Transformations
#'
#' These functions operate on one or more geography vectors and
#' return a geography vector.
#'
#' @inheritParams s2_is_collection
#' @param na.rm For aggregate calculations use `na.rm = TRUE`
#'   to drop missing values.
#' @param grid_size The grid size to which coordinates should be snapped;
#'   will be rounded to the nearest power of 10.
#' @param options An [s2_options()] object describing the polygon/polyline
#'   model to use and the snap level.
#' @param distance The distance to buffer, in units of `radius`.
#' @param max_cells The maximum number of cells to approximate a buffer.
#' @param min_level The minimum cell level used to approximate a buffer
#'   (1 - 30). Setting this value too high will result in unnecessarily
#'   large geographies, but may help improve buffers along long, narrow
#'   regions.
#' @param tolerance The minimum distance between vertexes to use when
#'   simplifying a geography.
#'
#' @inheritSection s2_options Model
#'
#' @export
#'
#' @seealso
#' BigQuery's geography function reference:
#'
#' - [ST_BOUNDARY](https://cloud.google.com/bigquery/docs/reference/standard-sql/geography_functions#st_boundary)
#' - [ST_CENTROID](https://cloud.google.com/bigquery/docs/reference/standard-sql/geography_functions#st_centroid)
#' - [ST_CLOSESTPOINT](https://cloud.google.com/bigquery/docs/reference/standard-sql/geography_functions#st_closestpoint)
#' - [ST_DIFFERENCE](https://cloud.google.com/bigquery/docs/reference/standard-sql/geography_functions#st_difference)
#' - [ST_INTERSECTION](https://cloud.google.com/bigquery/docs/reference/standard-sql/geography_functions#st_intersection)
#' - [ST_UNION](https://cloud.google.com/bigquery/docs/reference/standard-sql/geography_functions#st_union)
#' - [ST_SNAPTOGRID](https://cloud.google.com/bigquery/docs/reference/standard-sql/geography_functions#st_snaptogrid)
#' - [ST_SIMPLIFY](https://cloud.google.com/bigquery/docs/reference/standard-sql/geography_functions#st_simplify)
#' - [ST_UNION_AGG](https://cloud.google.com/bigquery/docs/reference/standard-sql/geography_functions#st_union_agg)
#' - [ST_CENTROID_AGG](https://cloud.google.com/bigquery/docs/reference/standard-sql/geography_functions#s2_centroid_agg)
#'
#' @examples
#' # returns the boundary:
#' # empty for point, endpoints of a linestring,
#' # perimeter of a polygon
#' s2_boundary("POINT (-64 45)")
#' s2_boundary("LINESTRING (0 0, 10 0)")
#' s2_boundary("POLYGON ((0 0, 10 0, 10 10, 0 10, 0 0))")
#'
#' # returns the area-weighted centroid, element-wise
#' s2_centroid("POLYGON ((0 0, 10 0, 10 10, 0 10, 0 0))")
#' s2_centroid("LINESTRING (0 0, 10 0)")
#'
#' # returns the unweighted centroid of the entire input
#' s2_centroid_agg(c("POINT (0 0)", "POINT (10 0)"))
#'
#' # returns the closest point on x to y
#' s2_closest_point(
#'   "POLYGON ((0 0, 10 0, 10 10, 0 10, 0 0))",
#'   "POINT (0 90)" # north pole!
#' )
#'
#' # returns the shortest possible line between x and y
#' s2_minimum_clearance_line_between(
#'   "POLYGON ((0 0, 10 0, 10 10, 0 10, 0 0))",
#'   "POINT (0 90)" # north pole!
#' )
#'
#' # binary operations: difference, symmetric difference, intersection and union
#' s2_difference(
#'   "POLYGON ((0 0, 10 0, 10 10, 0 10, 0 0))",
#'   "POLYGON ((5 5, 15 5, 15 15, 5 15, 5 5))",
#'   # 32 bit platforms may need to set snap rounding
#'   s2_options(snap = s2_snap_level(30))
#' )
#'
#' s2_sym_difference(
#'   "POLYGON ((0 0, 10 0, 10 10, 0 10, 0 0))",
#'   "POLYGON ((5 5, 15 5, 15 15, 5 15, 5 5))",
#'   # 32 bit platforms may need to set snap rounding
#'   s2_options(snap = s2_snap_level(30))
#' )
#'
#' s2_intersection(
#'   "POLYGON ((0 0, 10 0, 10 10, 0 10, 0 0))",
#'   "POLYGON ((5 5, 15 5, 15 15, 5 15, 5 5))",
#'   # 32 bit platforms may need to set snap rounding
#'   s2_options(snap = s2_snap_level(30))
#' )
#'
#' s2_union(
#'   "POLYGON ((0 0, 10 0, 10 10, 0 10, 0 0))",
#'   "POLYGON ((5 5, 15 5, 15 15, 5 15, 5 5))",
#'   # 32 bit platforms may need to set snap rounding
#'   s2_options(snap = s2_snap_level(30))
#' )
#'
#' # use s2_union_agg() to aggregate geographies in a vector
#' s2_union_agg(
#'   c(
#'     "POLYGON ((0 0, 10 0, 10 10, 0 10, 0 0))",
#'     "POLYGON ((5 5, 15 5, 15 15, 5 15, 5 5))"
#'   ),
#'   # 32 bit platforms may need to set snap rounding
#'   s2_options(snap = s2_snap_level(30))
#' )
#'
#' # snap to grid rounds coordinates to a specified grid size
#' s2_snap_to_grid("POINT (0.333333333333 0.666666666666)", 1e-2)
#'
s2_boundary <- function(x) {
  new_s2_xptr(cpp_s2_boundary(as_s2_geography(x)), "s2_geography")
}

#' @rdname s2_boundary
#' @export
s2_centroid <- function(x) {
  new_s2_xptr(cpp_s2_centroid(as_s2_geography(x)), "s2_geography")
}

#' @rdname s2_boundary
#' @export
s2_closest_point <- function(x, y) {
  recycled <- recycle_common(as_s2_geography(x), as_s2_geography(y))
  new_s2_xptr(cpp_s2_closest_point(recycled[[1]], recycled[[2]]), "s2_geography")
}

#' @rdname s2_boundary
#' @export
s2_minimum_clearance_line_between <- function(x, y) {
  recycled <- recycle_common(as_s2_geography(x), as_s2_geography(y))
  new_s2_xptr(cpp_s2_minimum_clearance_line_between(recycled[[1]], recycled[[2]]), "s2_geography")
}

#' @rdname s2_boundary
#' @export
s2_difference <- function(x, y, options = s2_options()) {
  recycled <- recycle_common(as_s2_geography(x), as_s2_geography(y))
  new_s2_xptr(cpp_s2_difference(recycled[[1]], recycled[[2]], options), "s2_geography")
}

#' @rdname s2_boundary
#' @export
s2_sym_difference <- function(x, y, options = s2_options()) {
  recycled <- recycle_common(as_s2_geography(x), as_s2_geography(y))
  new_s2_xptr(cpp_s2_sym_difference(recycled[[1]], recycled[[2]], options), "s2_geography")
}

#' @rdname s2_boundary
#' @export
s2_intersection <- function(x, y, options = s2_options()) {
  recycled <- recycle_common(as_s2_geography(x), as_s2_geography(y))
  new_s2_xptr(cpp_s2_intersection(recycled[[1]], recycled[[2]], options), "s2_geography")
}

#' @rdname s2_boundary
#' @export
s2_union <- function(x, y = NULL, options = s2_options()) {
  if (is.null(y)) {
    y <- as_s2_geography("POINT EMPTY")
  }

  recycled <- recycle_common(as_s2_geography(x), as_s2_geography(y))
  new_s2_xptr(cpp_s2_union(recycled[[1]], recycled[[2]], options), "s2_geography")
}

#' @rdname s2_boundary
#' @export
s2_snap_to_grid <- function(x, grid_size) {
  s2_rebuild(
    x,
    options = s2_options(
      snap = s2_snap_precision(10^(-log10(grid_size))),
      duplicate_edges = TRUE
    )
  )
}

#' @rdname s2_boundary
#' @export
s2_simplify <- function(x, tolerance, radius = s2_earth_radius_meters()) {
  s2_rebuild(x, options = s2_options(snap_radius = tolerance / radius, simplify_edge_chains = TRUE))
}

#' @rdname s2_boundary
#' @export
s2_rebuild <- function(x, options = s2_options()) {
  new_s2_xptr(cpp_s2_rebuild(as_s2_geography(x), options), "s2_geography")
}

#' @rdname s2_boundary
#' @export
s2_buffer_cells <- function(x, distance, max_cells = 1000, min_level = -1,
                            radius = s2_earth_radius_meters()) {
  recycled <- recycle_common(as_s2_geography(x), distance / radius)
  new_s2_xptr(cpp_s2_buffer_cells(recycled[[1]], recycled[[2]], max_cells, min_level), "s2_geography")
}

#' @rdname s2_boundary
#' @export
s2_centroid_agg <- function(x, na.rm = FALSE) {
  new_s2_xptr(cpp_s2_centroid_agg(as_s2_geography(x), naRm = na.rm), "s2_geography")
}

#' @rdname s2_boundary
#' @export
s2_union_agg <- function(x, options = s2_options(), na.rm = FALSE) {
  new_s2_xptr(cpp_s2_union_agg(as_s2_geography(x), options, na.rm), "s2_geography")
}
