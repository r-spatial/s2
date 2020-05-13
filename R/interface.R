
#' S2 Geography Accessors
#'
#' @param x,y An object that can be interpreted as an S2 type.
#'
#' @export
#'
#' @seealso
#' BigQuery's geography function reference:
#'
#' - [ST_ISCOLLECTION](https://cloud.google.com/bigquery/docs/reference/standard-sql/geography_functions#st_iscollection)
#' - [ST_DIMENSION](https://cloud.google.com/bigquery/docs/reference/standard-sql/geography_functions#st_dimension)
#' - [ST_NUMPOINTS](https://cloud.google.com/bigquery/docs/reference/standard-sql/geography_functions#st_numpoints)
#' - [ST_AREA](https://cloud.google.com/bigquery/docs/reference/standard-sql/geography_functions#st_area)
#' - [ST_LENGTH](https://cloud.google.com/bigquery/docs/reference/standard-sql/geography_functions#st_length)
#' - [ST_PERIMETER](https://cloud.google.com/bigquery/docs/reference/standard-sql/geography_functions#st_perimeter)
#' - [ST_X](https://cloud.google.com/bigquery/docs/reference/standard-sql/geography_functions#st_x)
#' - [ST_Y](https://cloud.google.com/bigquery/docs/reference/standard-sql/geography_functions#st_y)
#' - [ST_DISTANCE](https://cloud.google.com/bigquery/docs/reference/standard-sql/geography_functions#st_distance)
#' - [ST_MAXDISTANCE](https://cloud.google.com/bigquery/docs/reference/standard-sql/geography_functions#st_maxdistance)
#'
s2_iscollection <- function(x) {
  stop("Not implemented")
}

#' @rdname s2_iscollection
#' @export
s2_dimension <- function(x) {
  stop("Not implemented")
}

#' @rdname s2_iscollection
#' @export
s2_numpoints <- function(x) {
  stop("Not implemented")
}

#' @rdname s2_iscollection
#' @export
s2_area <- function(x) {
  stop("Not implemented")
}

#' @rdname s2_iscollection
#' @export
s2_length <- function(x) {
  stop("Not implemented")
}

#' @rdname s2_iscollection
#' @export
s2_perimeter <- function(x) {
  stop("Not implemented")
}

#' @rdname s2_iscollection
#' @export
s2_x <- function(x) {
  stop("Not implemented")
}

#' @rdname s2_iscollection
#' @export
s2_y <- function(x) {
  stop("Not implemented")
}

#' @rdname s2_iscollection
#' @export
s2_distance <- function(x, y) {
  stop("Not implemented")
}

#' @rdname s2_iscollection
#' @export
s2_maxdistance <- function(x, y) {
  stop("Not implemented")
}

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
