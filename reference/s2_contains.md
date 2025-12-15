# S2 Geography Predicates

These functions operate two geography vectors (pairwise), and return a
logical vector.

## Usage

``` r
s2_contains(x, y, options = s2_options(model = "open"))

s2_within(x, y, options = s2_options(model = "open"))

s2_covered_by(x, y, options = s2_options(model = "closed"))

s2_covers(x, y, options = s2_options(model = "closed"))

s2_disjoint(x, y, options = s2_options())

s2_intersects(x, y, options = s2_options())

s2_equals(x, y, options = s2_options())

s2_intersects_box(
  x,
  lng1,
  lat1,
  lng2,
  lat2,
  detail = 1000,
  options = s2_options()
)

s2_touches(x, y, options = s2_options())

s2_dwithin(x, y, distance, radius = s2_earth_radius_meters())

s2_prepared_dwithin(x, y, distance, radius = s2_earth_radius_meters())
```

## Arguments

- x, y:

  [geography
  vectors](https://r-spatial.github.io/s2/reference/as_s2_geography.md).
  These inputs are passed to
  [`as_s2_geography()`](https://r-spatial.github.io/s2/reference/as_s2_geography.md),
  so you can pass other objects (e.g., character vectors of well-known
  text) directly.

- options:

  An
  [`s2_options()`](https://r-spatial.github.io/s2/reference/s2_options.md)
  object describing the polygon/polyline model to use and the snap
  level.

- lng1, lat1, lng2, lat2:

  A latitude/longitude range

- detail:

  The number of points with which to approximate non-geodesic edges.

- distance:

  A distance on the surface of the earth in the same units as `radius`.

- radius:

  Radius of the earth. Defaults to the average radius of the earth in
  meters as defined by
  [`s2_earth_radius_meters()`](https://r-spatial.github.io/s2/reference/s2_earth_radius_meters.md).

## Model

The geometry model indicates whether or not a geometry includes its
boundaries. Boundaries of line geometries are its end points. OPEN
geometries do not contain their boundary (`model = "open"`); CLOSED
geometries (`model = "closed"`) contain their boundary; SEMI-OPEN
geometries (`model = "semi-open"`) contain half of their boundaries,
such that when two polygons do not overlap or two lines do not cross, no
point exist that belong to more than one of the geometries. (This latter
form, half-closed, is not present in the OpenGIS "simple feature access"
(SFA) standard nor DE9-IM on which that is based). The default values
for `s2_contains()` (open) and covers/covered_by (closed) correspond to
the SFA standard specification of these operators.

## See also

Matrix versions of these predicates (e.g.,
[`s2_intersects_matrix()`](https://r-spatial.github.io/s2/reference/s2_closest_feature.md)).

BigQuery's geography function reference:

- [ST_CONTAINS](https://cloud.google.com/bigquery/docs/reference/standard-sql/geography_functions#st_contains)

- [ST_COVEREDBY](https://cloud.google.com/bigquery/docs/reference/standard-sql/geography_functions#st_coveredby)

- [ST_COVERS](https://cloud.google.com/bigquery/docs/reference/standard-sql/geography_functions#st_covers)

- [ST_DISJOINT](https://cloud.google.com/bigquery/docs/reference/standard-sql/geography_functions#st_disjoint)

- [ST_EQUALS](https://cloud.google.com/bigquery/docs/reference/standard-sql/geography_functions#st_equals)

- [ST_INTERSECTS](https://cloud.google.com/bigquery/docs/reference/standard-sql/geography_functions#st_intersects)

- [ST_INTERSECTSBOX](https://cloud.google.com/bigquery/docs/reference/standard-sql/geography_functions#st_intersectsbox)

- [ST_TOUCHES](https://cloud.google.com/bigquery/docs/reference/standard-sql/geography_functions#st_touches)

- [ST_WITHIN](https://cloud.google.com/bigquery/docs/reference/standard-sql/geography_functions#st_within)

- [ST_DWITHIN](https://cloud.google.com/bigquery/docs/reference/standard-sql/geography_functions#st_dwithin)

## Examples

``` r
s2_contains(
  "POLYGON ((0 0, 10 0, 10 10, 0 10, 0 0))",
  c("POINT (5 5)", "POINT (-1 1)")
)
#> [1]  TRUE FALSE

s2_within(
  c("POINT (5 5)", "POINT (-1 1)"),
  "POLYGON ((0 0, 10 0, 10 10, 0 10, 0 0))"
)
#> [1]  TRUE FALSE

s2_covered_by(
  "POLYGON ((0 0, 10 0, 10 10, 0 10, 0 0))",
  c("POINT (5 5)", "POINT (-1 1)")
)
#> [1] FALSE FALSE

s2_covers(
  "POLYGON ((0 0, 10 0, 10 10, 0 10, 0 0))",
  c("POINT (5 5)", "POINT (-1 1)")
)
#> [1]  TRUE FALSE

s2_disjoint(
  "POLYGON ((0 0, 10 0, 10 10, 0 10, 0 0))",
  c("POINT (5 5)", "POINT (-1 1)")
)
#> [1] FALSE  TRUE

s2_intersects(
  "POLYGON ((0 0, 10 0, 10 10, 0 10, 0 0))",
  c("POINT (5 5)", "POINT (-1 1)")
)
#> [1]  TRUE FALSE

s2_equals(
  "POLYGON ((0 0, 10 0, 10 10, 0 10, 0 0))",
  c(
    "POLYGON ((0 0, 10 0, 10 10, 0 10, 0 0))",
    "POLYGON ((10 0, 10 10, 0 10, 0 0, 10 0))",
    "POLYGON ((-1 -1, 10 0, 10 10, 0 10, -1 -1))"
  )
)
#> [1]  TRUE  TRUE FALSE

s2_intersects(
  "POLYGON ((0 0, 10 0, 10 10, 0 10, 0 0))",
  c("POINT (5 5)", "POINT (-1 1)")
)
#> [1]  TRUE FALSE

s2_intersects_box(
  c("POINT (5 5)", "POINT (-1 1)"),
  0, 0, 10, 10
)
#> [1]  TRUE FALSE

s2_touches(
  "POLYGON ((0 0, 0 1, 1 1, 0 0))",
  c("POINT (0 0)", "POINT (0.5 0.75)", "POINT (0 0.5)")
)
#> [1]  TRUE FALSE FALSE

s2_dwithin(
  "POLYGON ((0 0, 10 0, 10 10, 0 10, 0 0))",
  c("POINT (5 5)", "POINT (-1 1)"),
  0 # distance in meters
)
#> [1]  TRUE FALSE

s2_dwithin(
  "POLYGON ((0 0, 10 0, 10 10, 0 10, 0 0))",
  c("POINT (5 5)", "POINT (-1 1)"),
  1e6 # distance in meters
)
#> [1] TRUE TRUE
```
