

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
