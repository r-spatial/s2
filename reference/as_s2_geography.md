# Create an S2 Geography Vector

Geography vectors are arrays of points, lines, polygons, and/or
collections of these. Geography vectors assume coordinates are longitude
and latitude on a perfect sphere.

## Usage

``` r
as_s2_geography(x, ...)

s2_geography()

# S3 method for class 's2_geography'
as_s2_geography(x, ...)

# S3 method for class 'wk_xy'
as_s2_geography(x, ...)

# S3 method for class 'wk_wkb'
as_s2_geography(x, ..., oriented = FALSE, check = TRUE)

# S3 method for class 'WKB'
as_s2_geography(x, ..., oriented = FALSE, check = TRUE)

# S3 method for class 'blob'
as_s2_geography(x, ..., oriented = FALSE, check = TRUE)

# S3 method for class 'wk_wkt'
as_s2_geography(x, ..., oriented = FALSE, check = TRUE)

# S3 method for class 'character'
as_s2_geography(x, ..., oriented = FALSE, check = TRUE)

# S3 method for class 'logical'
as_s2_geography(x, ...)

# S3 method for class 's2_geography'
as_wkb(x, ...)

# S3 method for class 's2_geography'
as_wkt(x, ...)
```

## Arguments

- x:

  An object that can be converted to an s2_geography vector

- ...:

  Unused

- oriented:

  TRUE if polygon ring directions are known to be correct (i.e.,
  exterior rings are defined counter clockwise and interior rings are
  defined clockwise).

- check:

  Use `check = FALSE` to skip error on invalid geometries

## Value

An object with class s2_geography

## Details

The coercion function `as_s2_geography()` is used to wrap the input of
most functions in the s2 package so that you can use other objects with
an unambiguious interpretation as a geography vector. Geography vectors
have a minimal
[vctrs](https://vctrs.r-lib.org/reference/vctrs-package.html)
implementation, so you can use these objects in tibble, dplyr, and other
packages that use the vctrs framework.

## See also

[`s2_geog_from_wkb()`](https://r-spatial.github.io/s2/reference/s2_geog_point.md),
[`s2_geog_from_text()`](https://r-spatial.github.io/s2/reference/s2_geog_point.md),
[`s2_geog_point()`](https://r-spatial.github.io/s2/reference/s2_geog_point.md),
[`s2_make_line()`](https://r-spatial.github.io/s2/reference/s2_geog_point.md),
[`s2_make_polygon()`](https://r-spatial.github.io/s2/reference/s2_geog_point.md)
for other ways to create geography vectors, and
[`s2_as_binary()`](https://r-spatial.github.io/s2/reference/s2_geog_point.md)
and
[`s2_as_text()`](https://r-spatial.github.io/s2/reference/s2_geog_point.md)
for other ways to export them.
