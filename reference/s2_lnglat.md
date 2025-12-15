# Create an S2 LngLat Vector

This class represents a latitude and longitude on the Earth's surface.
Most calculations in S2 convert this to a
[`as_s2_point()`](https://r-spatial.github.io/s2/reference/s2_point.md),
which is a unit vector representation of this value.

## Usage

``` r
s2_lnglat(lng, lat)

as_s2_lnglat(x, ...)

# Default S3 method
as_s2_lnglat(x, ...)

# S3 method for class 'wk_xy'
as_s2_lnglat(x, ...)

# S3 method for class 'wk_xyz'
as_s2_lnglat(x, ...)
```

## Arguments

- lat, lng:

  Vectors of latitude and longitude values in degrees.

- x:

  A `s2_lnglat()` vector or an object that can be coerced to one.

- ...:

  Unused

## Value

An object with class s2_lnglat

## Examples

``` r
s2_lnglat(45, -64) # Halifax, Nova Scotia!
#> <wk_xy[1] with CRS=OGC:CRS84>
#> [1] (45 -64)
as.data.frame(s2_lnglat(45, -64))
#>    x   y
#> 1 45 -64
```
