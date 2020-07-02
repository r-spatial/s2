
#' S2 Geography Accessors
#'
#' Accessors extract information about [geography vectors][as_s2_geography].
#'
#' @param x,y [geography vectors][as_s2_geography]. These inputs
#'   are passed to [as_s2_geography()], so you can pass other objects
#'   (e.g., character vectors of well-known text) directly.
#' @param radius Radius of the earth. Defaults to the average radius of
#'   the earth in meters as defined by [s2_earth_radius_meters()].
#'
#' @export
#'
#' @seealso
#' BigQuery's geography function reference:
#'
#' - [ST_ISCOLLECTION](https://cloud.google.com/bigquery/docs/reference/standard-sql/geography_functions#st_iscollection)
#' - [ST_DIMENSION](https://cloud.google.com/bigquery/docs/reference/standard-sql/geography_functions#st_dimension)
#' - [ST_NUMPOINTS](https://cloud.google.com/bigquery/docs/reference/standard-sql/geography_functions#st_numpoints)
#' - [ST_ISEMPTY](https://cloud.google.com/bigquery/docs/reference/standard-sql/geography_functions#st_isempty)
#' - [ST_AREA](https://cloud.google.com/bigquery/docs/reference/standard-sql/geography_functions#st_area)
#' - [ST_LENGTH](https://cloud.google.com/bigquery/docs/reference/standard-sql/geography_functions#st_length)
#' - [ST_PERIMETER](https://cloud.google.com/bigquery/docs/reference/standard-sql/geography_functions#st_perimeter)
#' - [ST_X](https://cloud.google.com/bigquery/docs/reference/standard-sql/geography_functions#st_x)
#' - [ST_Y](https://cloud.google.com/bigquery/docs/reference/standard-sql/geography_functions#st_y)
#' - [ST_DISTANCE](https://cloud.google.com/bigquery/docs/reference/standard-sql/geography_functions#st_distance)
#' - [ST_MAXDISTANCE](https://cloud.google.com/bigquery/docs/reference/standard-sql/geography_functions#st_maxdistance)
#'
#' @examples
#' # s2_is_collection() tests for multiple geometries in one feature
#' s2_is_collection(c("POINT (-64 45)", "MULTIPOINT ((-64 45), (8 72))"))
#'
#' # s2_dimension() returns 0 for point, 1  for line, 2 for polygon
#' s2_dimension(
#'   c(
#'     "GEOMETRYCOLLECTION EMPTY",
#'     "POINT (-64 45)",
#'     "LINESTRING (-64 45, 8 72)",
#'     "POLYGON ((0 0, 0 10, 10 10, 10 0, 0 0))",
#'     "GEOMETRYCOLLECTION (POINT (-64 45), LINESTRING (-64 45, 8 72))"
#'    )
#' )
#'
#' # s2_num_points() counts points
#' s2_num_points(c("POINT (-64 45)", "LINESTRING (-64 45, 8 72)"))
#'
#' # s2_is_empty tests for emptiness
#' s2_is_empty(c("POINT (-64 45)", "POINT EMPTY"))
#'
#' # calculate area, length, and perimeter
#' s2_area("POLYGON ((0 0, 0 10, 10 10, 10 0, 0 0))")
#' s2_perimeter("POLYGON ((0 0, 0 10, 10 10, 10 0, 0 0))")
#' s2_length(s2_boundary("POLYGON ((0 0, 0 10, 10 10, 10 0, 0 0))"))
#'
#' # extract x and y coordinates from points
#' s2_x(c("POINT (-64 45)", "POINT EMPTY"))
#' s2_y(c("POINT (-64 45)", "POINT EMPTY"))
#'
#' # calculate minimum and maximum distance between two geometries
#' s2_distance(
#'   "POLYGON ((0 0, 0 10, 10 10, 10 0, 0 0))",
#'   "POINT (-64 45)"
#' )
#' s2_max_distance(
#'   "POLYGON ((0 0, 0 10, 10 10, 10 0, 0 0))",
#'   "POINT (-64 45)"
#' )
#'
s2_is_collection <- function(x) {
  cpp_s2_is_collection(as_s2_geography(x))
}

#' @rdname s2_is_collection
#' @export
s2_dimension <- function(x) {
  cpp_s2_dimension(as_s2_geography(x))
}

#' @rdname s2_is_collection
#' @export
s2_num_points <- function(x) {
  cpp_s2_num_points(as_s2_geography(x))
}

#' @rdname s2_is_collection
#' @export
s2_is_empty <- function(x) {
  cpp_s2_is_empty(as_s2_geography(x))
}

#' @rdname s2_is_collection
#' @export
s2_area <- function(x, radius = s2_earth_radius_meters()) {
  recycled <- recycle_common(as_s2_geography(x), radius)
  cpp_s2_area(recycled[[1]]) * radius ^ 2
}

#' @rdname s2_is_collection
#' @export
s2_length <- function(x, radius = s2_earth_radius_meters()) {
  recycled <- recycle_common(as_s2_geography(x), radius)
  cpp_s2_length(recycled[[1]]) * radius
}

#' @rdname s2_is_collection
#' @export
s2_perimeter <- function(x, radius = s2_earth_radius_meters()) {
  recycled <- recycle_common(as_s2_geography(x), radius)
  cpp_s2_perimeter(recycled[[1]]) * radius
}

#' @rdname s2_is_collection
#' @export
s2_x <- function(x) {
  cpp_s2_x(as_s2_geography(x))
}

#' @rdname s2_is_collection
#' @export
s2_y <- function(x) {
  cpp_s2_y(as_s2_geography(x))
}

#' @rdname s2_is_collection
#' @export
s2_distance <- function(x, y, radius = s2_earth_radius_meters()) {
  recycled <- recycle_common(as_s2_geography(x), as_s2_geography(y), radius)
  cpp_s2_distance(recycled[[1]], recycled[[2]]) * radius
}

#' @rdname s2_is_collection
#' @export
s2_max_distance <- function(x, y, radius = s2_earth_radius_meters()) {
  recycled <- recycle_common(as_s2_geography(x), as_s2_geography(y), radius)
  cpp_s2_max_distance(recycled[[1]], recycled[[2]]) * radius
}
