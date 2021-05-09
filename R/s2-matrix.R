
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
#' @param k The number of closest edges to consider when searching. Note
#'   that in S2 a point is also considered an edge.
#' @param min_distance The minimum distance to consider when searching for
#'   edges. This filter is applied after the search is complete (i.e.,
#'   may cause fewer than `k` values to be returned).
#' @param max_edges_per_cell For [s2_may_intersect_matrix()],
#'   this values controls the nature of the index on `y`, with higher values
#'   leading to coarser index. Values should be between 10 and 50; the default
#'   of 50 is adequate for most use cases, but for specialized operations users
#'   may wish to use a lower value to increase performance.
#' @param max_feature_cells For [s2_may_intersect_matrix()], this value
#'   controls the approximation of `x` used to identify potential intersections
#'   on `y`. The default value of 4 gives the best performance for most operations,
#'   but for specialized operations users may wish to use a higher value to increase
#'   performance.
#'
#' @return A vector of length `x`.
#' @export
#'
#' @seealso
#' See pairwise predicate functions (e.g., [s2_intersects()]).
#'
#' @examples
#' city_names <- c("Vatican City", "San Marino", "Luxembourg")
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
#' # use s2_closest_edges() to find the k-nearest neighbours
#' nearest <- s2_closest_edges(cities, cities, k = 2, min_distance = 0)
#' city_names
#' city_names[unlist(nearest)]
#'
#' # predicate matrices
#' country_names[s2_intersects_matrix(cities, countries)[[1]]]
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
s2_closest_edges <- function(x, y, k, min_distance = -1, radius = s2_earth_radius_meters()) {
  stopifnot(k >= 1)
  cpp_s2_closest_edges(as_s2_geography(x), as_s2_geography(y), k, min_distance / radius)
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
s2_contains_matrix <- function(x, y, options = s2_options(model = "open")) {
  cpp_s2_contains_matrix(as_s2_geography(x), as_s2_geography(y), options)
}

#' @rdname s2_closest_feature
#' @export
s2_within_matrix <- function(x, y, options = s2_options(model = "open")) {
  cpp_s2_within_matrix(as_s2_geography(x), as_s2_geography(y), options)
}

#' @rdname s2_closest_feature
#' @export
s2_covers_matrix <- function(x, y, options = s2_options(model = "closed")) {
  cpp_s2_contains_matrix(as_s2_geography(x), as_s2_geography(y), options)
}

#' @rdname s2_closest_feature
#' @export
s2_covered_by_matrix <- function(x, y, options = s2_options(model = "closed")) {
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
  # disjoint is the odd one out, in that it requires a negation of intersects
  # this is inconvenient to do on the C++ level, and is easier to maintain
  # with setdiff() here (unless somebody complains that this is slow)
  intersection <- cpp_s2_intersects_matrix(as_s2_geography(x), as_s2_geography(y), options)
  Map(setdiff, list(seq_along(y)), intersection)
}

#' @rdname s2_closest_feature
#' @export
s2_equals_matrix <- function(x, y, options = s2_options()) {
  cpp_s2_equals_matrix(as_s2_geography(x), as_s2_geography(y), options)
}

#' @rdname s2_closest_feature
#' @export
s2_touches_matrix <- function(x, y, options = s2_options()) {
  cpp_s2_touches_matrix(as_s2_geography(x), as_s2_geography(y), options)
}

#' @rdname s2_closest_feature
#' @export
s2_dwithin_matrix <- function(x, y, distance, radius = s2_earth_radius_meters()) {
  cpp_s2_dwithin_matrix(as_s2_geography(x), as_s2_geography(y), distance / radius)
}

#' @rdname s2_closest_feature
#' @export
s2_may_intersect_matrix <- function(x, y, max_edges_per_cell = 50, max_feature_cells = 4) {
  cpp_s2_may_intersect_matrix(
    as_s2_geography(x), as_s2_geography(y),
    max_edges_per_cell, max_feature_cells,
    s2_options()
  )
}

# ------- for testing, non-indexed versions of matrix operators -------

s2_contains_matrix_brute_force <- function(x, y, options = s2_options()) {
  cpp_s2_contains_matrix_brute_force(as_s2_geography(x), as_s2_geography(y), options)
}

s2_within_matrix_brute_force <- function(x, y, options = s2_options()) {
  cpp_s2_within_matrix_brute_force(as_s2_geography(x), as_s2_geography(y), options)
}

s2_covers_matrix_brute_force <- function(x, y, options = s2_options(model = "closed")) {
  cpp_s2_contains_matrix_brute_force(as_s2_geography(x), as_s2_geography(y), options)
}

s2_covered_by_matrix_brute_force <- function(x, y, options = s2_options(model = "closed")) {
  cpp_s2_within_matrix_brute_force(as_s2_geography(x), as_s2_geography(y), options)
}

s2_intersects_matrix_brute_force <- function(x, y, options = s2_options()) {
  cpp_s2_intersects_matrix_brute_force(as_s2_geography(x), as_s2_geography(y), options)
}

s2_disjoint_matrix_brute_force <- function(x, y, options = s2_options()) {
  cpp_s2_disjoint_matrix_brute_force(as_s2_geography(x), as_s2_geography(y), options)
}

s2_equals_matrix_brute_force <- function(x, y, options = s2_options()) {
  cpp_s2_equals_matrix_brute_force(as_s2_geography(x), as_s2_geography(y), options)
}
