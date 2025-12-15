# Matrix Functions

These functions are similar to accessors and predicates, but instead of
recycling `x` and `y` to a common length and returning a vector of that
length, these functions return a vector of length `x` with each element
`i` containing information about how the entire vector `y` relates to
the feature at `x[i]`.

## Usage

``` r
s2_closest_feature(x, y)

s2_closest_edges(
  x,
  y,
  k,
  min_distance = -1,
  max_distance = Inf,
  radius = s2_earth_radius_meters()
)

s2_farthest_feature(x, y)

s2_distance_matrix(x, y, radius = s2_earth_radius_meters())

s2_max_distance_matrix(x, y, radius = s2_earth_radius_meters())

s2_contains_matrix(x, y, options = s2_options(model = "open"))

s2_within_matrix(x, y, options = s2_options(model = "open"))

s2_covers_matrix(x, y, options = s2_options(model = "closed"))

s2_covered_by_matrix(x, y, options = s2_options(model = "closed"))

s2_intersects_matrix(x, y, options = s2_options())

s2_disjoint_matrix(x, y, options = s2_options())

s2_equals_matrix(x, y, options = s2_options())

s2_touches_matrix(x, y, options = s2_options())

s2_dwithin_matrix(x, y, distance, radius = s2_earth_radius_meters())

s2_may_intersect_matrix(x, y, max_edges_per_cell = 50, max_feature_cells = 4)
```

## Arguments

- x, y:

  Geography vectors, coerced using
  [`as_s2_geography()`](https://r-spatial.github.io/s2/reference/as_s2_geography.md).
  `x` is considered the source, where as `y` is considered the target.

- k:

  The number of closest edges to consider when searching. Note that in
  S2 a point is also considered an edge.

- min_distance:

  The minimum distance to consider when searching for edges. This filter
  is applied after the search is complete (i.e., may cause fewer than
  `k` values to be returned).

- max_distance:

  The maximum distance to consider when searching for edges. This filter
  is applied before the search.

- radius:

  Radius of the earth. Defaults to the average radius of the earth in
  meters as defined by
  [`s2_earth_radius_meters()`](https://r-spatial.github.io/s2/reference/s2_earth_radius_meters.md).

- options:

  An
  [`s2_options()`](https://r-spatial.github.io/s2/reference/s2_options.md)
  object describing the polygon/polyline model to use and the snap
  level.

- distance:

  A distance on the surface of the earth in the same units as `radius`.

- max_edges_per_cell:

  For `s2_may_intersect_matrix()`, this values controls the nature of
  the index on `y`, with higher values leading to coarser index. Values
  should be between 10 and 50; the default of 50 is adequate for most
  use cases, but for specialized operations users may wish to use a
  lower value to increase performance.

- max_feature_cells:

  For `s2_may_intersect_matrix()`, this value controls the approximation
  of `x` used to identify potential intersections on `y`. The default
  value of 4 gives the best performance for most operations, but for
  specialized operations users may wish to use a higher value to
  increase performance.

## Value

A vector of length `x`.

## See also

See pairwise predicate functions (e.g.,
[`s2_intersects()`](https://r-spatial.github.io/s2/reference/s2_contains.md)).

## Examples

``` r
city_names <- c("Vatican City", "San Marino", "Luxembourg")
cities <- s2_data_cities(city_names)
country_names <- s2_data_tbl_countries$name
countries <- s2_data_countries()

# closest feature returns y indices of the closest feature
# for each feature in x
country_names[s2_closest_feature(cities, countries)]
#> [1] "Italy"      "Italy"      "Luxembourg"

# farthest feature returns y indices of the farthest feature
# for each feature in x
country_names[s2_farthest_feature(cities, countries)]
#> [1] "New Zealand" "New Zealand" "New Zealand"

# use s2_closest_edges() to find the k-nearest neighbours
nearest <- s2_closest_edges(cities, cities, k = 2, min_distance = 0)
city_names
#> [1] "Vatican City" "San Marino"   "Luxembourg"  
city_names[unlist(nearest)]
#> [1] "San Marino"   "Vatican City" "San Marino"  

# predicate matrices
country_names[s2_intersects_matrix(cities, countries)[[1]]]
#> [1] "Italy"

# distance matrices
s2_distance_matrix(cities, cities)
#>          [,1]     [,2]     [,3]
#> [1,]      0.0 226040.9 986762.6
#> [2,] 226040.9      0.0 792700.1
#> [3,] 986762.6 792700.1      0.0
s2_max_distance_matrix(cities, countries[1:4])
#>         [,1]    [,2]      [,3]    [,4]
#> [1,] 5284474 6716222  729253.1 4488169
#> [2,] 5225941 6939199  798367.2 4551290
#> [3,] 5543951 7659266 1564998.8 5198533
```
