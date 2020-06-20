
#' Matrix functions
#'
#' These functions are similar to accessors and predicates, but instead of
#' recycling `x` and `y` to a common length and returning a vector of that
#' length, these functions return a vector of length `x` with each element
#' `i` containing information about how the entire vector `y` relates to
#' the feature at `x[i]`.
#'
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
#' country_names[s2_closest_feature(cities, countries)]
#'
s2_closest_feature <- function(x, y) {
  cpp_s2_closest_feature(as_s2_geography(x), as_s2_geography(y))
}
