# S2 Geography Transformations

These functions operate on one or more geography vectors and return a
geography vector.

## Usage

``` r
s2_boundary(x)

s2_centroid(x)

s2_closest_point(x, y)

s2_minimum_clearance_line_between(x, y)

s2_difference(x, y, options = s2_options())

s2_sym_difference(x, y, options = s2_options())

s2_intersection(x, y, options = s2_options())

s2_union(x, y = NULL, options = s2_options())

s2_snap_to_grid(x, grid_size)

s2_simplify(x, tolerance, radius = s2_earth_radius_meters())

s2_rebuild(x, options = s2_options())

s2_buffer_cells(
  x,
  distance,
  max_cells = 1000,
  min_level = -1,
  radius = s2_earth_radius_meters()
)

s2_convex_hull(x)

s2_centroid_agg(x, na.rm = FALSE)

s2_coverage_union_agg(x, options = s2_options(), na.rm = FALSE)

s2_rebuild_agg(x, options = s2_options(), na.rm = FALSE)

s2_union_agg(x, options = s2_options(), na.rm = FALSE)

s2_convex_hull_agg(x, na.rm = FALSE)

s2_point_on_surface(x, na.rm = FALSE)
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

- grid_size:

  The grid size to which coordinates should be snapped; will be rounded
  to the nearest power of 10.

- tolerance:

  The minimum distance between vertexes to use when simplifying a
  geography.

- radius:

  Radius of the earth. Defaults to the average radius of the earth in
  meters as defined by
  [`s2_earth_radius_meters()`](https://r-spatial.github.io/s2/reference/s2_earth_radius_meters.md).

- distance:

  The distance to buffer, in units of `radius`.

- max_cells:

  The maximum number of cells to approximate a buffer.

- min_level:

  The minimum cell level used to approximate a buffer (1 - 30). Setting
  this value too high will result in unnecessarily large geographies,
  but may help improve buffers along long, narrow regions.

- na.rm:

  For aggregate calculations use `na.rm = TRUE` to drop missing values.

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
for
[`s2_contains()`](https://r-spatial.github.io/s2/reference/s2_contains.md)
(open) and covers/covered_by (closed) correspond to the SFA standard
specification of these operators.

## See also

BigQuery's geography function reference:

- [ST_BOUNDARY](https://cloud.google.com/bigquery/docs/reference/standard-sql/geography_functions#st_boundary)

- [ST_CENTROID](https://cloud.google.com/bigquery/docs/reference/standard-sql/geography_functions#st_centroid)

- [ST_CLOSESTPOINT](https://cloud.google.com/bigquery/docs/reference/standard-sql/geography_functions#st_closestpoint)

- [ST_DIFFERENCE](https://cloud.google.com/bigquery/docs/reference/standard-sql/geography_functions#st_difference)

- [ST_INTERSECTION](https://cloud.google.com/bigquery/docs/reference/standard-sql/geography_functions#st_intersection)

- [ST_UNION](https://cloud.google.com/bigquery/docs/reference/standard-sql/geography_functions#st_union)

- [ST_SNAPTOGRID](https://cloud.google.com/bigquery/docs/reference/standard-sql/geography_functions#st_snaptogrid)

- [ST_SIMPLIFY](https://cloud.google.com/bigquery/docs/reference/standard-sql/geography_functions#st_simplify)

- [ST_UNION_AGG](https://cloud.google.com/bigquery/docs/reference/standard-sql/geography_functions#st_union_agg)

- [ST_CENTROID_AGG](https://cloud.google.com/bigquery/docs/reference/standard-sql/geography_functions#s2_centroid_agg)

## Examples

``` r
# returns the boundary:
# empty for point, endpoints of a linestring,
# perimeter of a polygon
s2_boundary("POINT (-64 45)")
#> <geodesic s2_geography[1] with CRS=OGC:CRS84>
#> [1] GEOMETRYCOLLECTION EMPTY
s2_boundary("LINESTRING (0 0, 10 0)")
#> <geodesic s2_geography[1] with CRS=OGC:CRS84>
#> [1] MULTIPOINT ((0 0), (10 0))
s2_boundary("POLYGON ((0 0, 10 0, 10 10, 0 10, 0 0))")
#> <geodesic s2_geography[1] with CRS=OGC:CRS84>
#> [1] LINESTRING (0 0, 10 0, 10 10, 0 10, 0 0...

# returns the area-weighted centroid, element-wise
s2_centroid("POLYGON ((0 0, 10 0, 10 10, 0 10, 0 0))")
#> <geodesic s2_geography[1] with CRS=OGC:CRS84>
#> [1] POINT (5 5.00595863)
s2_centroid("LINESTRING (0 0, 10 0)")
#> <geodesic s2_geography[1] with CRS=OGC:CRS84>
#> [1] POINT (5 0)

# s2_point_on_surface guarantees a point on surface
# Note: this is not the same as st_point_on_surface
s2_centroid("POLYGON ((0 0, 10 0, 1 1, 0 10, 0 0))")
#> <geodesic s2_geography[1] with CRS=OGC:CRS84>
#> [1] POINT (2.00377112 2.00234355)
s2_point_on_surface("POLYGON ((0 0, 10 0, 1 1, 0 10, 0 0))")
#> <geodesic s2_geography[1] with CRS=OGC:CRS84>
#> [1] POINT (0.450236802 0.450222902)

# returns the unweighted centroid of the entire input
s2_centroid_agg(c("POINT (0 0)", "POINT (10 0)"))
#> <geodesic s2_geography[1] with CRS=OGC:CRS84>
#> [1] POINT (5 0)

# returns the closest point on x to y
s2_closest_point(
  "POLYGON ((0 0, 10 0, 10 10, 0 10, 0 0))",
  "POINT (0 90)" # north pole!
)
#> <geodesic s2_geography[1] with CRS=OGC:CRS84>
#> [1] POINT (5 10.037423)

# returns the shortest possible line between x and y
s2_minimum_clearance_line_between(
  "POLYGON ((0 0, 10 0, 10 10, 0 10, 0 0))",
  "POINT (0 90)" # north pole!
)
#> <geodesic s2_geography[1] with CRS=OGC:CRS84>
#> [1] LINESTRING (5 10.037423, 0 90)

# binary operations: difference, symmetric difference, intersection and union
s2_difference(
  "POLYGON ((0 0, 10 0, 10 10, 0 10, 0 0))",
  "POLYGON ((5 5, 15 5, 15 15, 5 15, 5 5))",
  # 32 bit platforms may need to set snap rounding
  s2_options(snap = s2_snap_level(30))
)
#> <geodesic s2_geography[1] with CRS=OGC:CRS84>
#> [1] POLYGON ((5.00000004 10.0374231, 3.55739019e-08 10, 3.55739019e-08 3.55739019e-08, 10 3.50334544e-08, 10 5.01900178...

s2_sym_difference(
  "POLYGON ((0 0, 10 0, 10 10, 0 10, 0 0))",
  "POLYGON ((5 5, 15 5, 15 15, 5 15, 5 5))",
  # 32 bit platforms may need to set snap rounding
  s2_options(snap = s2_snap_level(30))
)
#> <geodesic s2_geography[1] with CRS=OGC:CRS84>
#> [1] MULTIPOLYGON (((5.00000004 10.0374231, 3.55739019e-08 10, 3.55739019e-08 3.55739019e-08, 10 3.50334544e-08, 10 5.01900178...

s2_intersection(
  "POLYGON ((0 0, 10 0, 10 10, 0 10, 0 0))",
  "POLYGON ((5 5, 15 5, 15 15, 5 15, 5 5))",
  # 32 bit platforms may need to set snap rounding
  s2_options(snap = s2_snap_level(30))
)
#> <geodesic s2_geography[1] with CRS=OGC:CRS84>
#> [1] POLYGON ((5.00000004 4.99999997, 10 5.01900178, 10 9.99999999, 5.00000004 10.0374231, 5.00000004 4.99999997...

s2_union(
  "POLYGON ((0 0, 10 0, 10 10, 0 10, 0 0))",
  "POLYGON ((5 5, 15 5, 15 15, 5 15, 5 5))",
  # 32 bit platforms may need to set snap rounding
  s2_options(snap = s2_snap_level(30))
)
#> <geodesic s2_geography[1] with CRS=OGC:CRS84>
#> [1] POLYGON ((5.00000004 10.0374231, 3.55739019e-08 10, 3.55739019e-08 3.55739019e-08, 10 3.50334544e-08, 10 5.01900178...

# s2_convex_hull_agg builds the convex hull of a list of geometries
s2_convex_hull_agg(
  c(
    "POLYGON ((0 0, 10 0, 10 10, 0 10, 0 0))",
    "POLYGON ((5 5, 15 5, 15 15, 5 15, 5 5))"
  )
)
#> <geodesic s2_geography[1] with CRS=OGC:CRS84>
#> [1] POLYGON ((0 0, 10 0, 15 5, 15 15, 5 15...

# use s2_union_agg() to aggregate geographies in a vector
s2_coverage_union_agg(
  c(
    "POLYGON ((0 0, 10 0, 10 10, 0 10, 0 0))",
    "POLYGON ((5 5, 15 5, 15 15, 5 15, 5 5))"
  ),
  # 32 bit platforms may need to set snap rounding
  s2_options(snap = s2_snap_level(30))
)
#> <geodesic s2_geography[1] with CRS=OGC:CRS84>
#> [1] MULTIPOLYGON (((3.55739019e-08 3.55739019e-08, 10 3.50334544e-08, 10 9.99999999, 3.55739019e-08 10, 3.55739019e-08 3.55739019e-08...

# snap to grid rounds coordinates to a specified grid size
s2_snap_to_grid("POINT (0.333333333333 0.666666666666)", 1e-2)
#> <geodesic s2_geography[1] with CRS=OGC:CRS84>
#> [1] POINT (0.33 0.67)

```
