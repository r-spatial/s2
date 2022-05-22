
#' S2 Geography Predicates
#'
#' These functions operate two geography vectors (pairwise), and return
#' a logical vector.
#'
#' @inheritParams s2_is_collection
#' @inheritParams s2_boundary
#' @param distance A distance on the surface of the earth in the same units
#'   as `radius`.
#' @param lng1,lat1,lng2,lat2 A latitude/longitude range
#' @param detail The number of points with which to approximate
#'   non-geodesic edges.
#'
#' @inheritSection s2_options Model
#'
#' @export
#'
#' @seealso
#' Matrix versions of these predicates (e.g., [s2_intersects_matrix()]).
#'
#' BigQuery's geography function reference:
#'
#' - [ST_CONTAINS](https://cloud.google.com/bigquery/docs/reference/standard-sql/geography_functions#st_contains)
#' - [ST_COVEREDBY](https://cloud.google.com/bigquery/docs/reference/standard-sql/geography_functions#st_coveredby)
#' - [ST_COVERS](https://cloud.google.com/bigquery/docs/reference/standard-sql/geography_functions#st_covers)
#' - [ST_DISJOINT](https://cloud.google.com/bigquery/docs/reference/standard-sql/geography_functions#st_disjoint)
#' - [ST_EQUALS](https://cloud.google.com/bigquery/docs/reference/standard-sql/geography_functions#st_equals)
#' - [ST_INTERSECTS](https://cloud.google.com/bigquery/docs/reference/standard-sql/geography_functions#st_intersects)
#' - [ST_INTERSECTSBOX](https://cloud.google.com/bigquery/docs/reference/standard-sql/geography_functions#st_intersectsbox)
#' - [ST_TOUCHES](https://cloud.google.com/bigquery/docs/reference/standard-sql/geography_functions#st_touches)
#' - [ST_WITHIN](https://cloud.google.com/bigquery/docs/reference/standard-sql/geography_functions#st_within)
#' - [ST_DWITHIN](https://cloud.google.com/bigquery/docs/reference/standard-sql/geography_functions#st_dwithin)
#'
#' @examples
#' s2_contains(
#'   "POLYGON ((0 0, 10 0, 10 10, 0 10, 0 0))",
#'   c("POINT (5 5)", "POINT (-1 1)")
#' )
#'
#' s2_within(
#'   c("POINT (5 5)", "POINT (-1 1)"),
#'   "POLYGON ((0 0, 10 0, 10 10, 0 10, 0 0))"
#' )
#'
#' s2_covered_by(
#'   "POLYGON ((0 0, 10 0, 10 10, 0 10, 0 0))",
#'   c("POINT (5 5)", "POINT (-1 1)")
#' )
#'
#' s2_covers(
#'   "POLYGON ((0 0, 10 0, 10 10, 0 10, 0 0))",
#'   c("POINT (5 5)", "POINT (-1 1)")
#' )
#'
#' s2_disjoint(
#'   "POLYGON ((0 0, 10 0, 10 10, 0 10, 0 0))",
#'   c("POINT (5 5)", "POINT (-1 1)")
#' )
#'
#' s2_intersects(
#'   "POLYGON ((0 0, 10 0, 10 10, 0 10, 0 0))",
#'   c("POINT (5 5)", "POINT (-1 1)")
#' )
#'
#' s2_equals(
#'   "POLYGON ((0 0, 10 0, 10 10, 0 10, 0 0))",
#'   c(
#'     "POLYGON ((0 0, 10 0, 10 10, 0 10, 0 0))",
#'     "POLYGON ((10 0, 10 10, 0 10, 0 0, 10 0))",
#'     "POLYGON ((-1 -1, 10 0, 10 10, 0 10, -1 -1))"
#'   )
#' )
#'
#' s2_intersects(
#'   "POLYGON ((0 0, 10 0, 10 10, 0 10, 0 0))",
#'   c("POINT (5 5)", "POINT (-1 1)")
#' )
#'
#' s2_intersects_box(
#'   c("POINT (5 5)", "POINT (-1 1)"),
#'   0, 0, 10, 10
#' )
#'
#' s2_touches(
#'   "POLYGON ((0 0, 0 1, 1 1, 0 0))",
#'   c("POINT (0 0)", "POINT (0.5 0.75)", "POINT (0 0.5)")
#' )
#'
#' s2_dwithin(
#'   "POLYGON ((0 0, 10 0, 10 10, 0 10, 0 0))",
#'   c("POINT (5 5)", "POINT (-1 1)"),
#'   0 # distance in meters
#' )
#'
#' s2_dwithin(
#'   "POLYGON ((0 0, 10 0, 10 10, 0 10, 0 0))",
#'   c("POINT (5 5)", "POINT (-1 1)"),
#'   1e6 # distance in meters
#' )
#'
s2_contains <- function(x, y, options = s2_options(model = "open")) {
  recycled <- recycle_common(as_s2_geography(x), as_s2_geography(y))
  cpp_s2_contains(recycled[[1]], recycled[[2]], options)
}

#' @rdname s2_contains
#' @export
s2_within <- function(x, y, options = s2_options(model = "open")) {
  s2_contains(y, x, options)
}

#' @rdname s2_contains
#' @export
s2_covered_by <- function(x, y, options = s2_options(model = "closed")) {
  s2_covers(y, x, options)
}

#' @rdname s2_contains
#' @export
s2_covers <- function(x, y, options = s2_options(model = "closed")) {
  recycled <- recycle_common(as_s2_geography(x), as_s2_geography(y))
  cpp_s2_contains(recycled[[1]], recycled[[2]], options)
}

#' @rdname s2_contains
#' @export
s2_disjoint <- function(x, y, options = s2_options()) {
  !s2_intersects(x, y, options)
}

#' @rdname s2_contains
#' @export
s2_intersects <- function(x, y, options = s2_options()) {
  recycled <- recycle_common(as_s2_geography(x), as_s2_geography(y))
  cpp_s2_intersects(recycled[[1]], recycled[[2]], options)
}

#' @rdname s2_contains
#' @export
s2_equals <- function(x, y, options = s2_options()) {
  recycled <- recycle_common(as_s2_geography(x), as_s2_geography(y))
  cpp_s2_equals(recycled[[1]], recycled[[2]], options)
}

#' @rdname s2_contains
#' @export
s2_intersects_box <- function(x, lng1, lat1, lng2, lat2, detail = 1000, options = s2_options()) {
  recycled <- recycle_common(as_s2_geography(x), lng1, lat1, lng2, lat2, detail)
  cpp_s2_intersects_box(
    recycled[[1]],
    recycled[[2]], recycled[[3]],
    recycled[[4]], recycled[[5]],
    detail = recycled[[6]],
	  s2options = options
  )
}

#' @rdname s2_contains
#' @export
s2_touches <- function(x, y, options = s2_options()) {
  recycled <- recycle_common(as_s2_geography(x), as_s2_geography(y))
  cpp_s2_touches(recycled[[1]], recycled[[2]], options)
}

#' @rdname s2_contains
#' @export
s2_dwithin <- function(x, y, distance, radius = s2_earth_radius_meters()) {
  recycled <- recycle_common(as_s2_geography(x), as_s2_geography(y), distance / radius)
  cpp_s2_dwithin(recycled[[1]], recycled[[2]], recycled[[3]])
}

#' @rdname s2_contains
#' @export
s2_prepared_dwithin <- function(x, y, distance, radius = s2_earth_radius_meters()) {
  recycled <- recycle_common(as_s2_geography(x), as_s2_geography(y), distance / radius)
  cpp_s2_prepared_dwithin(recycled[[1]], recycled[[2]], recycled[[3]])
}
