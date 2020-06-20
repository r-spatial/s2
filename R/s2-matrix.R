
#' Matrix functions
#'
#' These functions are similar to accessors and predicates, but instead of
#' recycling `x` and `y` to a common length and returning a vector of that
#' length, these functions return a vector of length `x` with each element
#' `i` containing information about how the entire vector `y` relates to
#' the feature at `x[i]`.
#'
#' @inheritParams s2_is_collection
#' @param x,y Geography vectors, coerced using [as_s2_geography()].
#'   `x` is considered the source, where as `y` is considered the target.
#'
#' @return A vector of length `x`.
#' @export
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
#' # distance matrices
#' s2_distance_matrix(cities, cities)
#' s2_max_distance_matrix(cities, countries[1:4])
#'
s2_closest_feature <- function(x, y) {
  cpp_s2_closest_feature(as_s2_geography(x), as_s2_geography(y))
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
