

#' S2 Geography Transformations
#'
#' @inheritParams s2_iscollection
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
#' - [ST_SNAPTOGRID](https://cloud.google.com/bigquery/docs/reference/standard-sql/geography_functions#st_snaptogrid)
#' - [ST_UNION](https://cloud.google.com/bigquery/docs/reference/standard-sql/geography_functions#st_union)
#'
s2_boundary <- function(x) {
  stop("Not implemented")
}

#' @rdname s2_boundary
#' @export
s2_centroid <- function(x) {
  stop("Not implemented")
}

#' @rdname s2_boundary
#' @export
s2_snaptogrid <- function(x) {
  stop("Not implemented")
}

#' @rdname s2_boundary
#' @export
s2_union <- function(x, y = NULL) {
  stop("Not implemented")
}

#' @rdname s2_boundary
#' @export
s2_closestpoint <- function(x, y) {
  stop("Not implemented")
}

#' @rdname s2_boundary
#' @export
s2_difference <- function(x, y) {
  stop("Not implemented")
}

#' @rdname s2_boundary
#' @export
s2_intersection <- function(x, y) {
  stop("Not implemented")
}

#' @rdname s2_boundary
#' @export
s2_union_agg <- function(x) {
  stop("Not implemented")
}

#' @rdname s2_boundary
#' @export
s2_centroid_agg <- function(x) {
  stop("Not implemented")
}

#' S2 Geography Predicates
#'
#' @inheritParams s2_iscollection
#' @param lng1,lat1,lng2,lat2 A latitude/longitude range
#' @param distance A distance in meters on the surface of the earth
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
#' - [ST_ISEMPTY](https://cloud.google.com/bigquery/docs/reference/standard-sql/geography_functions#st_isempty)
#' - [ST_DWITHIN](https://cloud.google.com/bigquery/docs/reference/standard-sql/geography_functions#st_dwithin)
#'
s2_contains <- function(x, y) {
  stop("Not implemented")
}

#' @rdname s2_contains
#' @export
s2_coveredby <- function(x, y) {
  stop("Not implemented")
}

#' @rdname s2_contains
#' @export
s2_covers <- function(x, y) {
  stop("Not implemented")
}

#' @rdname s2_contains
#' @export
s2_disjoint <- function(x, y) {
  stop("Not implemented")
}

#' @rdname s2_contains
#' @export
s2_equals <- function(x, y) {
  stop("Not implemented")
}

#' @rdname s2_contains
#' @export
s2_intersects <- function(x) {
  stop("Not implemented")
}

#' @rdname s2_contains
#' @export
s2_intersectsbox <- function(x, lng1, lat1, lng2, lat2) {
  stop("Not implemented")
}

#' @rdname s2_contains
#' @export
s2_touches <- function(x) {
  stop("Not implemented")
}

#' @rdname s2_contains
#' @export
s2_within <- function(x, y) {
  stop("Not implemented")
}

#' @rdname s2_contains
#' @export
s2_isempty <- function(x, y) {
  stop("Not implemented")
}

#' @rdname s2_contains
#' @export
s2_dwithin <- function(x, y, distance) {
  stop("Not implemented")
}
