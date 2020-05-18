
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
#' - [ST_DWITHIN](https://cloud.google.com/bigquery/docs/reference/standard-sql/geography_functions#st_dwithin)
#'
s2_contains <- function(x, y) {
  recycled <- recycle_common(s2geography(x), s2geography(y))
  libs2_cpp_s2_contains(recycled[[1]], recycled[[2]])
}

#' @rdname s2_contains
#' @export
s2_coveredby <- function(x, y) {
  s2_covers(y, x)
}

#' @rdname s2_contains
#' @export
s2_covers <- function(x, y) {
  stop("Not implemented")
}

#' @rdname s2_contains
#' @export
s2_disjoint <- function(x, y) {
  !s2_intersects(x, y)
}

#' @rdname s2_contains
#' @export
s2_equals <- function(x, y) {
  recycled <- recycle_common(s2geography(x), s2geography(y))
  libs2_cpp_s2_equals(recycled[[1]], recycled[[2]])
}

#' @rdname s2_contains
#' @export
s2_intersects <- function(x, y) {
  recycled <- recycle_common(s2geography(x), s2geography(y))
  libs2_cpp_s2_intersects(recycled[[1]], recycled[[2]])
}

#' @rdname s2_contains
#' @export
s2_intersectsbox <- function(x, lng1, lat1, lng2, lat2) {
  recycled <- recycle_common(s2geography(x), lng1, lat1, lng2, lat2)
  libs2_cpp_s2_intersectsbox(
    recycled[[1]],
    recycled[[2]], recycled[[3]],
    recycled[[4]], recycled[[5]]
  )
}

#' @rdname s2_contains
#' @export
s2_touches <- function(x, y) {
  stop("Not implemented")
}

#' @rdname s2_contains
#' @export
s2_within <- function(x, y) {
  s2_contains(y, x)
}

#' @rdname s2_contains
#' @export
s2_dwithin <- function(x, y, distance, radius = s2earth_radius_meters()) {
  recycled <- recycle_common(s2geography(x), s2geography(y), distance / radius)
  libs2_cpp_s2_dwithin(recycled[[1]], recycled[[2]], recycled[[3]])
}
