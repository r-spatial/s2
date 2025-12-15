# Create an S2 Point Vector

In S2 terminology, a "point" is a 3-dimensional unit vector
representation of an `s2_point()`. Internally, all s2 objects are stored
as 3-dimensional unit vectors.

## Usage

``` r
s2_point(x, y, z)

s2_point_crs()

as_s2_point(x, ...)

# Default S3 method
as_s2_point(x, ...)

# S3 method for class 'wk_xy'
as_s2_point(x, ...)

# S3 method for class 'wk_xyz'
as_s2_point(x, ...)
```

## Arguments

- x, y, z:

  Vectors of latitude and longitude values in degrees.

- ...:

  Unused

## Value

An object with class s2_point

## Examples

``` r
point <- s2_lnglat(-64, 45) # Halifax, Nova Scotia!
as_s2_point(point)
#> <wk_xyz[1] with CRS=s2_point_crs>
#> [1] Z (0.3099752 -0.6355434 0.7071068)
as.data.frame(as_s2_point(point))
#>           x          y         z
#> 1 0.3099752 -0.6355434 0.7071068
```
