# Linear referencing

Linear referencing

## Usage

``` r
s2_project(x, y, radius = s2_earth_radius_meters())

s2_project_normalized(x, y)

s2_interpolate(x, distance, radius = s2_earth_radius_meters())

s2_interpolate_normalized(x, distance_normalized)
```

## Arguments

- x:

  A simple polyline geography vector

- y:

  A simple point geography vector. The point will be snapped to the
  nearest point on `x` for the purposes of interpolation.

- radius:

  Radius of the earth. Defaults to the average radius of the earth in
  meters as defined by
  [`s2_earth_radius_meters()`](https://r-spatial.github.io/s2/reference/s2_earth_radius_meters.md).

- distance:

  A distance along `x` in `radius` units.

- distance_normalized:

  A `distance` normalized to
  [`s2_length()`](https://r-spatial.github.io/s2/reference/s2_is_collection.md)
  of `x`.

## Value

- `s2_interpolate()` returns the point on `x`, `distance` meters along
  the line.

- `s2_interpolate_normalized()` returns the point on `x` interpolated to
  a fraction along the line.

- `s2_project()` returns the `distance` that `point` occurs along `x`.

- `s2_project_normalized()` returns the `distance_normalized` along `x`
  where `point` occurs.

## Examples

``` r
s2_project_normalized("LINESTRING (0 0, 0 90)", "POINT (0 22.5)")
#> [1] 0.25
s2_project("LINESTRING (0 0, 0 90)", "POINT (0 22.5)")
#> [1] 2501890
s2_interpolate_normalized("LINESTRING (0 0, 0 90)", 0.25)
#> <geodesic s2_geography[1] with CRS=OGC:CRS84>
#> [1] POINT (0 22.5)
s2_interpolate("LINESTRING (0 0, 0 90)", 2501890)
#> <geodesic s2_geography[1] with CRS=OGC:CRS84>
#> [1] POINT (0 22.500002)
```
