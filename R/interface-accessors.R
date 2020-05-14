
#' S2 Geography Accessors
#'
#' @param x,y An object that can be interpreted as an S2 type.
#' @param radius Radius of the earth. Defaults to the average radius of
#'   the earth as defined by [s2earth_radius_meters()].
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
  libs2_cpp_s2_iscollection(s2geography(x))
}

#' @rdname s2_iscollection
#' @export
s2_dimension <- function(x) {
  libs2_cpp_s2_dimension(s2geography(x))
}

#' @rdname s2_iscollection
#' @export
s2_numpoints <- function(x) {
  libs2_cpp_s2_numpoints(s2geography(x))
}

#' @rdname s2_iscollection
#' @export
s2_area <- function(x) {
  libs2_cpp_s2_area(s2geography(x))
}

#' @rdname s2_iscollection
#' @export
s2_length <- function(x) {
  libs2_cpp_s2_length(s2geography(x))
}

#' @rdname s2_iscollection
#' @export
s2_perimeter <- function(x) {
  libs2_cpp_s2_perimeter(s2geography(x))
}

#' @rdname s2_iscollection
#' @export
s2_x <- function(x) {
  libs2_cpp_s2_x(s2geography(x))
}

#' @rdname s2_iscollection
#' @export
s2_y <- function(x) {
  libs2_cpp_s2_y(s2geography(x))
}

#' @rdname s2_iscollection
#' @export
s2_distance <- function(x, y, radius = s2earth_radius_meters()) {
  recycled <- recycle_common(s2geography(x), s2geography(y))
  libs2_cpp_s2_distance(recycled[[1]], recycled[[2]]) * radius
}

#' @rdname s2_iscollection
#' @export
s2_maxdistance <- function(x, y) {
  stop("Not implemented")
}
