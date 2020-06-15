
#' Create and format geography vectors
#'
#' @inheritParams s2_iscollection
#' @inheritParams as_s2_geography
#' @param longitude,latitude Vectors of latitude and longitude
#' @param wkt_string Well-known text
#' @param wkb_bytes A `list()` of `raw()`
#' @param endian 0 for big endian and 1 for little endian.
#'
#' @export
#'
#' @seealso
#' BigQuery's geography function reference:
#'
#' - [ST_GEOGPOINT](https://cloud.google.com/bigquery/docs/reference/standard-sql/geography_functions#st_geogpoint)
#' - [ST_MAKELINE](https://cloud.google.com/bigquery/docs/reference/standard-sql/geography_functions#st_makeline)
#' - [ST_MAKEPOLYGON](https://cloud.google.com/bigquery/docs/reference/standard-sql/geography_functions#st_makepolygon)
#' - [ST_MAKEPOLYGONORIENTED](https://cloud.google.com/bigquery/docs/reference/standard-sql/geography_functions#st_makepolygonoriented)
#' - [ST_GEOGFROMTEXT](https://cloud.google.com/bigquery/docs/reference/standard-sql/geography_functions#st_geogfromtext)
#' - [ST_GEOGFROMWKB](https://cloud.google.com/bigquery/docs/reference/standard-sql/geography_functions#st_geogfromwkb)
#' - [ST_ASTEXT](https://cloud.google.com/bigquery/docs/reference/standard-sql/geography_functions#st_astext)
#' - [ST_ASBINARY](https://cloud.google.com/bigquery/docs/reference/standard-sql/geography_functions#st_asbinary)
#'
s2_geogpoint <- function(longitude, latitude) {
  stop("Not implemented")
}

#' @rdname s2_geogpoint
#' @export
s2_makeline <- function(longitude, latitude) {
  stop("Not implemented")
}

#' @rdname s2_geogpoint
#' @export
s2_makepolygon <- function(longitude, latitude) {
  stop("Not implemented")
}

#' @rdname s2_geogpoint
#' @export
s2_makepolygonoriented <- function(longitude, latitude) {
  stop("Not implemented")
}

#' @rdname s2_geogpoint
#' @export
s2_geogfromtext <- function(wkt_string, oriented = FALSE) {
  stop("Not implemented")
}

#' @rdname s2_geogpoint
#' @export
s2_geogfromwkb <- function(wkb_bytes, oriented = FALSE) {
  stop("Not implemented")
}

#' @rdname s2_geogpoint
#' @export
s2_astext <- function(x) {
  s2_geography_to_wkt(as_s2_geography(x), precision = 16, trim = TRUE)
}

#' @rdname s2_geogpoint
#' @export
s2_asbinary <- function(x, endian = match(.Platform$endian, c("big", "little")) - 1) {
  s2_geography_to_wkb(as_s2_geography(x), endian = endian)
}
