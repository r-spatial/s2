# Compute feature-wise and aggregate bounds

`s2_bounds_rect()` returns a bounding latitude-longitude rectangle that
contains the region; `s2_bounds_cap()` returns a bounding circle
represented by a centre point (lat, lng) and an angle. The bound may not
be tight for points, polylines and geometry collections. The rectangle
returned may depend on the order of points or polylines. `lng_lo` values
larger than `lng_hi` indicate regions that span the antimeridian, see
the Fiji example.

## Usage

``` r
s2_bounds_cap(x)

s2_bounds_rect(x)
```

## Arguments

- x:

  An
  [`s2_geography()`](https://r-spatial.github.io/s2/reference/as_s2_geography.md)
  vector.

## Value

Both functions return a `data.frame`:

- `s2_bounds_rect()`: Columns `minlng`, `minlat`, `maxlng`, `maxlat`
  (degrees)

- `s2_bounds_cap()`: Columns `lng`, `lat`, `angle` (degrees)

## Examples

``` r
s2_bounds_cap(s2_data_countries("Antarctica"))
#>   lng lat    angle
#> 1   0 -90 26.72934
s2_bounds_cap(s2_data_countries("Netherlands"))
#>        lng      lat    angle
#> 1 5.203512 52.15706 1.792879
s2_bounds_cap(s2_data_countries("Fiji"))
#>        lng       lat    angle
#> 1 178.7459 -17.15444 1.801369

s2_bounds_rect(s2_data_countries("Antarctica"))
#>   lng_lo lat_lo lng_hi    lat_hi
#> 1   -180    -90    180 -63.27066
s2_bounds_rect(s2_data_countries("Netherlands"))
#>     lng_lo   lat_lo   lng_hi  lat_hi
#> 1 3.314971 50.80372 7.092053 53.5104
s2_bounds_rect(s2_data_countries("Fiji"))
#>    lng_lo    lat_lo    lng_hi    lat_hi
#> 1 177.285 -18.28799 -179.7933 -16.02088
```
