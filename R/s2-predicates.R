
#' S2 Geography Predicates
#'
#' @inheritParams s2_is_collection
#' @inheritParams s2_boundary
#' @param lng1,lat1,lng2,lat2 A latitude/longitude range
#' @param distance A distance in meters on the surface of the earth
#' @param detail The number of points with which to approximate
#'   non-geodesic edges.
#'
#' @inheritSection s2_model_default Model
#'
#' @export
#'
#' @seealso
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
s2_contains <- function(x, y, model = 0) {
  recycled <- recycle_common(as_s2_geography(x), as_s2_geography(y))
  cpp_s2_contains(recycled[[1]], recycled[[2]], model = model)
}

#' @rdname s2_contains
#' @export
s2_covered_by <- function(x, y, model = 2) {
  s2_covers(y, x, model = model)
}

#' @rdname s2_contains
#' @export
s2_covers <- function(x, y, model = 2) {
  recycled <- recycle_common(as_s2_geography(x), as_s2_geography(y))
  cpp_s2_contains(recycled[[1]], recycled[[2]], model = model)
}

#' @rdname s2_contains
#' @export
s2_disjoint <- function(x, y, model = s2_model_default()) {
  !s2_intersects(x, y, model = model)
}

#' @rdname s2_contains
#' @export
s2_equals <- function(x, y, model = s2_model_default()) {
  recycled <- recycle_common(as_s2_geography(x), as_s2_geography(y))
  cpp_s2_equals(recycled[[1]], recycled[[2]], model)
}

#' @rdname s2_contains
#' @export
s2_intersects <- function(x, y, model = s2_model_default()) {
  recycled <- recycle_common(as_s2_geography(x), as_s2_geography(y))
  cpp_s2_intersects(recycled[[1]], recycled[[2]], model)
}

#' @rdname s2_contains
#' @export
s2_intersects_box <- function(x, lng1, lat1, lng2, lat2, detail = 1000, model = s2_model_default()) {
  recycled <- recycle_common(as_s2_geography(x), lng1, lat1, lng2, lat2, detail)
  cpp_s2_intersects_box(
    recycled[[1]],
    recycled[[2]], recycled[[3]],
    recycled[[4]], recycled[[5]],
    detail = recycled[[6]],
	  model
  )
}

#' @rdname s2_contains
#' @export
s2_touches <- function(x, y) {
  stop("Not implemented")
}

#' @rdname s2_contains
#' @export
s2_within <- function(x, y, model = 0) {
  s2_contains(y, x, model = model)
}

#' @rdname s2_contains
#' @export
s2_dwithin <- function(x, y, distance, radius = s2_earth_radius_meters()) {
  recycled <- recycle_common(as_s2_geography(x), as_s2_geography(y), distance / radius)
  cpp_s2_dwithin(recycled[[1]], recycled[[2]], recycled[[3]])
}
