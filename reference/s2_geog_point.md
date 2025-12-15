# Create and Format Geography Vectors

These functions create and export [geography
vectors](https://r-spatial.github.io/s2/reference/as_s2_geography.md).
Unlike the BigQuery geography constructors, these functions do not
sanitize invalid or redundant input using
[`s2_union()`](https://r-spatial.github.io/s2/reference/s2_boundary.md).
Note that when creating polygons using `s2_make_polygon()`, rings can be
open or closed.

## Usage

``` r
s2_geog_point(longitude, latitude)

s2_make_line(longitude, latitude, feature_id = 1L)

s2_make_polygon(
  longitude,
  latitude,
  feature_id = 1L,
  ring_id = 1L,
  oriented = FALSE,
  check = TRUE
)

s2_geog_from_text(
  wkt_string,
  oriented = FALSE,
  check = TRUE,
  planar = FALSE,
  tessellate_tol_m = s2_tessellate_tol_default()
)

s2_geog_from_wkb(
  wkb_bytes,
  oriented = FALSE,
  check = TRUE,
  planar = FALSE,
  tessellate_tol_m = s2_tessellate_tol_default()
)

s2_as_text(
  x,
  precision = 16,
  trim = TRUE,
  planar = FALSE,
  tessellate_tol_m = s2_tessellate_tol_default()
)

s2_as_binary(
  x,
  endian = wk::wk_platform_endian(),
  planar = FALSE,
  tessellate_tol_m = s2_tessellate_tol_default()
)

s2_tessellate_tol_default()
```

## Arguments

- longitude, latitude:

  Vectors of latitude and longitude

- feature_id, ring_id:

  Vectors for which a change in sequential values indicates a new
  feature or ring. Use [`factor()`](https://rdrr.io/r/base/factor.html)
  to convert from a character vector.

- oriented:

  TRUE if polygon ring directions are known to be correct (i.e.,
  exterior rings are defined counter clockwise and interior rings are
  defined clockwise).

- check:

  Use `check = FALSE` to skip error on invalid geometries

- wkt_string:

  Well-known text

- planar:

  Use `TRUE` to force planar edges in import or export.

- tessellate_tol_m:

  The maximum number of meters to that a point must be moved to satisfy
  the planar edge constraint.

- wkb_bytes:

  A [`list()`](https://rdrr.io/r/base/list.html) of
  [`raw()`](https://rdrr.io/r/base/raw.html)

- x:

  An object that can be converted to an s2_geography vector

- precision:

  The number of significant digits to export when writing well-known
  text. If `trim = FALSE`, the number of digits after the decimal place.

- trim:

  Should trailing zeroes be included after the decimal place?

- endian:

  The endian-ness of the well-known binary. See
  [`wk::wkb_translate_wkb()`](https://paleolimbot.github.io/wk/reference/deprecated.html).

## See also

See
[`as_s2_geography()`](https://r-spatial.github.io/s2/reference/as_s2_geography.md)
for other ways to construct geography vectors.

BigQuery's geography function reference:

- [ST_GEOGPOINT](https://cloud.google.com/bigquery/docs/reference/standard-sql/geography_functions#st_geogpoint)

- [ST_MAKELINE](https://cloud.google.com/bigquery/docs/reference/standard-sql/geography_functions#st_makeline)

- [ST_MAKEPOLYGON](https://cloud.google.com/bigquery/docs/reference/standard-sql/geography_functions#st_makepolygon)

- [ST_GEOGFROMTEXT](https://cloud.google.com/bigquery/docs/reference/standard-sql/geography_functions#st_geogfromtext)

- [ST_GEOGFROMWKB](https://cloud.google.com/bigquery/docs/reference/standard-sql/geography_functions#st_geogfromwkb)

- [ST_ASTEXT](https://cloud.google.com/bigquery/docs/reference/standard-sql/geography_functions#st_astext)

- [ST_ASBINARY](https://cloud.google.com/bigquery/docs/reference/standard-sql/geography_functions#st_asbinary)

## Examples

``` r
# create point geographies using coordinate values:
s2_geog_point(-64, 45)
#> <geodesic s2_geography[1] with CRS=OGC:CRS84>
#> [1] POINT (-64 45)

# create line geographies using coordinate values:
s2_make_line(c(-64, 8), c(45, 71))
#> <geodesic s2_geography[1] with CRS=OGC:CRS84>
#> [1] LINESTRING (-64 45, 8 71)

# optionally, separate features using feature_id:
s2_make_line(
  c(-64, 8, -27, -27), c(45, 71, 0, 45),
  feature_id = c(1, 1, 2, 2)
)
#> <geodesic s2_geography[2] with CRS=OGC:CRS84>
#> [1] LINESTRING (-64 45, 8 71)  LINESTRING (-27 0, -27 45)

# create polygon geographies using coordinate values:
# (rings can be open or closed)
s2_make_polygon(c(-45, 8, 0), c(64, 71, 90))
#> <geodesic s2_geography[1] with CRS=OGC:CRS84>
#> [1] POLYGON ((-45 64, 8 71, 0 90, -45 64))

# optionally, separate rings and/or features using
# ring_id and/or feature_id
s2_make_polygon(
  c(20, 10, 10, 30, 45, 30, 20, 20, 40, 20, 45),
  c(35, 30, 10, 5, 20, 20, 15, 25, 40, 45, 30),
  feature_id = c(rep(1, 8), rep(2, 3)),
  ring_id = c(1, 1, 1, 1, 1, 2, 2, 2, 1, 1, 1)
)
#> <geodesic s2_geography[2] with CRS=OGC:CRS84>
#> [1] POLYGON ((20 35, 10 30, 10 10, 30 5, 45 20...
#> [2] POLYGON ((40 40, 20 45, 45 30, 40 40))       

# import and export well-known text
(geog <- s2_geog_from_text("POINT (-64 45)"))
#> <geodesic s2_geography[1] with CRS=OGC:CRS84>
#> [1] POINT (-64 45)
s2_as_text(geog)
#> [1] "POINT (-64 45)"

# import and export well-known binary
(geog <- s2_geog_from_wkb(wk::as_wkb("POINT (-64 45)")))
#> <geodesic s2_geography[1] with CRS=OGC:CRS84>
#> [1] POINT (-64 45)
s2_as_binary(geog)
#> [[1]]
#>  [1] 01 01 00 00 00 00 00 00 00 00 00 50 c0 00 00 00 00 00 80 46 40
#> 
#> attr(,"class")
#> [1] "blob"

# import geometry from planar space
s2_geog_from_text(
   "POLYGON ((0 0, 1 0, 0 1, 0 0))",
   planar = TRUE,
   tessellate_tol_m = 1
)
#> <geodesic s2_geography[1] with CRS=OGC:CRS84>
#> [1] POLYGON ((0 0, 1 0, 0.5 0.5, 0.25 0.75, 0 1...

# export geographies into planar space
geog <- s2_make_polygon(c(179, -179, 179), c(10, 10, 11))
s2_as_text(geog, planar = TRUE)
#> [1] "POLYGON ((179 10, 180 10.00149252698408, 181 10, 180.0016175935936 10.50156386165192, 179 11, 179 10))"

# polygons containing a pole need an extra step
geog <- s2_data_countries("Antarctica")
geom <- s2_as_text(
  s2_intersection(geog, s2_world_plate_carree()),
  planar = TRUE
)
```
