# S2 Geography Accessors

Accessors extract information about [geography
vectors](https://r-spatial.github.io/s2/reference/as_s2_geography.md).

## Usage

``` r
s2_is_collection(x)

s2_is_valid(x)

s2_is_valid_detail(x)

s2_dimension(x)

s2_num_points(x)

s2_is_empty(x)

s2_area(x, radius = s2_earth_radius_meters())

s2_length(x, radius = s2_earth_radius_meters())

s2_perimeter(x, radius = s2_earth_radius_meters())

s2_x(x)

s2_y(x)

s2_distance(x, y, radius = s2_earth_radius_meters())

s2_max_distance(x, y, radius = s2_earth_radius_meters())
```

## Arguments

- x, y:

  [geography
  vectors](https://r-spatial.github.io/s2/reference/as_s2_geography.md).
  These inputs are passed to
  [`as_s2_geography()`](https://r-spatial.github.io/s2/reference/as_s2_geography.md),
  so you can pass other objects (e.g., character vectors of well-known
  text) directly.

- radius:

  Radius of the earth. Defaults to the average radius of the earth in
  meters as defined by
  [`s2_earth_radius_meters()`](https://r-spatial.github.io/s2/reference/s2_earth_radius_meters.md).

## See also

BigQuery's geography function reference:

- [ST_ISCOLLECTION](https://cloud.google.com/bigquery/docs/reference/standard-sql/geography_functions#st_iscollection)

- [ST_DIMENSION](https://cloud.google.com/bigquery/docs/reference/standard-sql/geography_functions#st_dimension)

- [ST_NUMPOINTS](https://cloud.google.com/bigquery/docs/reference/standard-sql/geography_functions#st_numpoints)

- [ST_ISEMPTY](https://cloud.google.com/bigquery/docs/reference/standard-sql/geography_functions#st_isempty)

- [ST_AREA](https://cloud.google.com/bigquery/docs/reference/standard-sql/geography_functions#st_area)

- [ST_LENGTH](https://cloud.google.com/bigquery/docs/reference/standard-sql/geography_functions#st_length)

- [ST_PERIMETER](https://cloud.google.com/bigquery/docs/reference/standard-sql/geography_functions#st_perimeter)

- [ST_X](https://cloud.google.com/bigquery/docs/reference/standard-sql/geography_functions#st_x)

- [ST_Y](https://cloud.google.com/bigquery/docs/reference/standard-sql/geography_functions#st_y)

- [ST_DISTANCE](https://cloud.google.com/bigquery/docs/reference/standard-sql/geography_functions#st_distance)

- [ST_MAXDISTANCE](https://cloud.google.com/bigquery/docs/reference/standard-sql/geography_functions#st_maxdistance)

## Examples

``` r
# s2_is_collection() tests for multiple geometries in one feature
s2_is_collection(c("POINT (-64 45)", "MULTIPOINT ((-64 45), (8 72))"))
#> [1] FALSE  TRUE

# s2_dimension() returns 0 for point, 1  for line, 2 for polygon
s2_dimension(
  c(
    "GEOMETRYCOLLECTION EMPTY",
    "POINT (-64 45)",
    "LINESTRING (-64 45, 8 72)",
    "POLYGON ((0 0, 0 10, 10 10, 10 0, 0 0))",
    "GEOMETRYCOLLECTION (POINT (-64 45), LINESTRING (-64 45, 8 72))"
   )
)
#> [1] -1  0  1  2  1

# s2_num_points() counts points
s2_num_points(c("POINT (-64 45)", "LINESTRING (-64 45, 8 72)"))
#> [1] 1 2

# s2_is_empty tests for emptiness
s2_is_empty(c("POINT (-64 45)", "POINT EMPTY"))
#> [1] FALSE  TRUE

# calculate area, length, and perimeter
s2_area("POLYGON ((0 0, 0 10, 10 10, 10 0, 0 0))")
#> [1] 1.233205e+12
s2_perimeter("POLYGON ((0 0, 0 10, 10 10, 10 0, 0 0))")
#> [1] 4430869
s2_length(s2_boundary("POLYGON ((0 0, 0 10, 10 10, 10 0, 0 0))"))
#> [1] 4430869

# extract x and y coordinates from points
s2_x(c("POINT (-64 45)", "POINT EMPTY"))
#> [1] -64 NaN
s2_y(c("POINT (-64 45)", "POINT EMPTY"))
#> [1]  45 NaN

# calculate minimum and maximum distance between two geometries
s2_distance(
  "POLYGON ((0 0, 0 10, 10 10, 10 0, 0 0))",
  "POINT (-64 45)"
)
#> [1] 7189418
s2_max_distance(
  "POLYGON ((0 0, 0 10, 10 10, 10 0, 0 0))",
  "POINT (-64 45)"
)
#> [1] 8757818
```
