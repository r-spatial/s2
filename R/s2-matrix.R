
#' Matrix Functions
#'
#' These functions are similar to accessors and predicates, but instead of
#' recycling `x` and `y` to a common length and returning a vector of that
#' length, these functions return a vector of length `x` with each element
#' `i` containing information about how the entire vector `y` relates to
#' the feature at `x[i]`.
#'
#' @inheritParams s2_is_collection
#' @inheritParams s2_contains
#' @param x,y Geography vectors, coerced using [as_s2_geography()].
#'   `x` is considered the source, where as `y` is considered the target.
#'
#' @return A vector of length `x`.
#' @export
#'
#' @seealso
#' See pairwise predicate functions (e.g., [s2_intersects()]).
#'
#' @examples
#' city_names <- c("Vatican City", "San Marino", "Luxembourg", "Palikir")
#' cities <- s2_data_cities(city_names)
#' country_names <- s2_data_tbl_countries$name
#' countries <- s2_data_countries()
#'
#' # closest feature returns y indices of the closest feature
#' # for each feature in x
#' country_names[s2_closest_feature(cities, countries)]
#'
#' # farthest feature returns y indices of the farthest feature
#' # for each feature in x
#' country_names[s2_farthest_feature(cities, countries)]
#'
#' # distance matrices
#' s2_distance_matrix(cities, cities)
#' s2_max_distance_matrix(cities, countries[1:4])
#'
s2_closest_feature <- function(x, y) {
  cpp_s2_closest_feature(as_s2_geography(x), as_s2_geography(y))
}

#' @rdname s2_closest_feature
#' @export
s2_farthest_feature <- function(x, y) {
  cpp_s2_farthest_feature(as_s2_geography(x), as_s2_geography(y))
}

#' @rdname s2_closest_feature
#' @export
s2_distance_matrix <- function(x, y, radius = s2_earth_radius_meters()) {
  cpp_s2_distance_matrix(as_s2_geography(x), as_s2_geography(y)) * radius
}

#' @rdname s2_closest_feature
#' @export
s2_max_distance_matrix <- function(x, y, radius = s2_earth_radius_meters()) {
  cpp_s2_max_distance_matrix(as_s2_geography(x), as_s2_geography(y)) * radius
}

#' @rdname s2_closest_feature
#' @export
s2_contains_matrix <- function(x, y, options = s2_options()) {
  cpp_s2_contains_matrix(as_s2_geography(x), as_s2_geography(y), options)
}

#' @rdname s2_closest_feature
#' @export
s2_within_matrix <- function(x, y, options = s2_options()) {
  cpp_s2_within_matrix(as_s2_geography(x), as_s2_geography(y), options)
}

#' @rdname s2_closest_feature
#' @export
s2_covers_matrix <- function(x, y, options = s2_options(model = 2)) {
  cpp_s2_contains_matrix(as_s2_geography(x), as_s2_geography(y), options)
}

#' @rdname s2_closest_feature
#' @export
s2_covered_by_matrix <- function(x, y, options = s2_options(model = 2)) {
  cpp_s2_within_matrix(as_s2_geography(x), as_s2_geography(y), options)
}

#' @rdname s2_closest_feature
#' @export
s2_intersects_matrix <- function(x, y, options = s2_options()) {
  cpp_s2_intersects_matrix(as_s2_geography(x), as_s2_geography(y), options)
}

#' @rdname s2_closest_feature
#' @export
s2_disjoint_matrix <- function(x, y, options = s2_options()) {
  cpp_s2_disjoint_matrix(as_s2_geography(x), as_s2_geography(y), options)
}

#' @rdname s2_closest_feature
#' @export
s2_equals_matrix <- function(x, y, options = s2_options()) {
  cpp_s2_equals_matrix(as_s2_geography(x), as_s2_geography(y), options)
}

#' @rdname s2_closest_feature
#' @export
s2_touches_matrix <- function(x, y, options = s2_options()) {
  x <- as_s2_geography(x)
  y <- as_s2_geography(y)

  options_closed <- options
  options_closed$polygon_model <- 2
  options_closed$polyline_model <- 2

  options_open <- options
  options_open$polygon_model <- 0
  options_open$polyline_model <- 0

  intersects_closed <- cpp_s2_intersects_matrix(x, y, options_closed)
  intersects_open <- cpp_s2_intersects_matrix(x, y, options_open)
  Map(setdiff, intersects_closed, intersects_open)
}

#' @rdname s2_closest_feature
#' @export
s2_dwithin_matrix <- function(x, y, distance, radius = s2_earth_radius_meters()) {
  cpp_s2_dwithin_matrix(as_s2_geography(x), as_s2_geography(y), distance / radius)
}
