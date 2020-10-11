
<!-- README.md is generated from README.Rmd. Please edit that file -->

# s2

<!-- badges: start -->

[![Lifecycle:
experimental](https://img.shields.io/badge/lifecycle-experimental-orange.svg)](https://www.tidyverse.org/lifecycle/#experimental)
![R-CMD-check](https://github.com/r-spatial/s2/workflows/R-CMD-check/badge.svg)
[![codecov](https://codecov.io/gh/r-spatial/s2/branch/master/graph/badge.svg)](https://codecov.io/gh/r-spatial/s2)
[![CRAN](http://www.r-pkg.org/badges/version/s2)](https://cran.r-project.org/package=s2)
<!-- badges: end -->

The goal of s2 is to provide R bindings to Google’s
[S2Geometry](https://s2geometry.io) library. The package exposes an API
similar to Google’s [BigQuery Geography
API](https://cloud.google.com/bigquery/docs/reference/standard-sql/geography_functions),
whose functions also operate on spherical geometries. This package is a
complete rewrite of an earlier CRAN package s2 with versions up to
0.4-2, for which the sources are found
[here](https://github.com/spatstat/s2/).

## Installation

You can install the released version of s2 from
[CRAN](https://CRAN.R-project.org) with:

``` r
install.packages("s2")
```

And the development version from [GitHub](https://github.com/) with:

``` r
# install.packages("remotes")
remotes::install_github("r-spatial/s2")
```

## Example

The s2 package provides geometry transformers and predicates similar to
those found in [GEOS](https://trac.osgeo.org/geos/), except instead of
assuming a planar geometry, s2’s functions work in latitude and
longitude and assume a spherical geometry:

``` r
library(s2)

s2_contains(
  # polygon containing much  of the northern hemisphere
  "POLYGON ((-63.5 44.6, -149.75 61.20, 116.4 40.2, 13.5 52.51, -63.5 44.6))",
  # ...should contain the north pole
  "POINT (0 90)"
)
#> [1] TRUE
```

The [sf package](https://r-spatial.github.io/sf/) will soon support s2
natively, but s2 can also read well-known text and well-known binary:

``` r
library(dplyr)
library(sf)

nc_s2 <- read_sf(system.file("shape/nc.shp", package = "sf")) %>% 
  mutate(geometry = s2_geog_from_wkb(st_as_binary(geometry))) %>% 
  as_tibble() %>% 
  select(NAME, geometry)

nc_s2
#> # A tibble: 100 x 2
#>    NAME        geometry                                                         
#>    <chr>       <s2_geography>                                                   
#>  1 Ashe        <POLYGON ((-81.4529 36.2396, -81.431 36.2607, -81.4123 36.2673, …
#>  2 Alleghany   <POLYGON ((-81.1767 36.4154, -81.1534 36.4247, -81.1384 36.4176,…
#>  3 Surry       <POLYGON ((-80.453 36.2571, -80.4353 36.551, -80.6111 36.5573, -…
#>  4 Currituck   <MULTIPOLYGON (((-75.9419 36.2943, -75.9575 36.2595, -75.9138 36…
#>  5 Northampton <POLYGON ((-77.142 36.4171, -77.1393 36.4565, -77.1273 36.4707, …
#>  6 Hertford    <POLYGON ((-76.7075 36.2661, -76.7413 36.3152, -76.9241 36.3924,…
#>  7 Camden      <POLYGON ((-76.0173 36.3377, -76.0329 36.336, -76.044 36.3536, -…
#>  8 Gates       <POLYGON ((-76.4604 36.3739, -76.5025 36.4523, -76.4983 36.5039,…
#>  9 Warren      <POLYGON ((-78.1347 36.2366, -78.1096 36.2135, -78.0583 36.2113,…
#> 10 Stokes      <POLYGON ((-80.0241 36.545, -80.0481 36.5471, -80.4353 36.551, -…
#> # … with 90 more rows
```

Use accessors to extract information about geometries:

``` r
nc_s2 %>% 
  mutate(
    area = s2_area(geometry),
    perimeter = s2_perimeter(geometry)
  )
#> # A tibble: 100 x 4
#>    NAME      geometry                                             area perimeter
#>    <chr>     <s2_geography>                                      <dbl>     <dbl>
#>  1 Ashe      <POLYGON ((-81.4529 36.2396, -81.431 36.2607, -…   1.14e9   141627.
#>  2 Alleghany <POLYGON ((-81.1767 36.4154, -81.1534 36.4247, …   6.11e8   119876.
#>  3 Surry     <POLYGON ((-80.453 36.2571, -80.4353 36.551, -8…   1.42e9   160458.
#>  4 Currituck <MULTIPOLYGON (((-75.9419 36.2943, -75.9575 36.…   6.94e8   301644.
#>  5 Northamp… <POLYGON ((-77.142 36.4171, -77.1393 36.4565, -…   1.52e9   211794.
#>  6 Hertford  <POLYGON ((-76.7075 36.2661, -76.7413 36.3152, …   9.68e8   160780.
#>  7 Camden    <POLYGON ((-76.0173 36.3377, -76.0329 36.336, -…   6.16e8   150430.
#>  8 Gates     <POLYGON ((-76.4604 36.3739, -76.5025 36.4523, …   9.03e8   123170.
#>  9 Warren    <POLYGON ((-78.1347 36.2366, -78.1096 36.2135, …   1.18e9   141073.
#> 10 Stokes    <POLYGON ((-80.0241 36.545, -80.0481 36.5471, -…   1.23e9   140583.
#> # … with 90 more rows
```

Use predicates to subset vectors:

``` r
nc_s2 %>% 
  filter(s2_contains(geometry, "POINT (-80.9313 35.6196)"))
#> # A tibble: 1 x 2
#>   NAME    geometry                                                              
#>   <chr>   <s2_geography>                                                        
#> 1 Catawba <POLYGON ((-80.9313 35.6196, -81.0036 35.6971, -81.0548 35.7134, -81.…
```

Use transformers to create new geometries:

``` r
nc_s2 %>% 
  mutate(geometry = s2_boundary(geometry))
#> # A tibble: 100 x 2
#>    NAME        geometry                                                         
#>    <chr>       <s2_geography>                                                   
#>  1 Ashe        <LINESTRING (-81.4529 36.2396, -81.431 36.2607, -81.4123 36.2673…
#>  2 Alleghany   <LINESTRING (-81.1767 36.4154, -81.1534 36.4247, -81.1384 36.417…
#>  3 Surry       <LINESTRING (-80.453 36.2571, -80.4353 36.551, -80.6111 36.5573,…
#>  4 Currituck   <MULTILINESTRING ((-75.9419 36.2943, -75.9575 36.2595, -75.9138 …
#>  5 Northampton <LINESTRING (-77.142 36.4171, -77.1393 36.4565, -77.1273 36.4707…
#>  6 Hertford    <LINESTRING (-76.7075 36.2661, -76.7413 36.3152, -76.9241 36.392…
#>  7 Camden      <LINESTRING (-76.0173 36.3377, -76.0329 36.336, -76.044 36.3536,…
#>  8 Gates       <LINESTRING (-76.4604 36.3739, -76.5025 36.4523, -76.4983 36.503…
#>  9 Warren      <LINESTRING (-78.1347 36.2366, -78.1096 36.2135, -78.0583 36.211…
#> 10 Stokes      <LINESTRING (-80.0241 36.545, -80.0481 36.5471, -80.4353 36.551,…
#> # … with 90 more rows
```

Finally, use the WKB or WKT exporters to export to sf or some other
package:

``` r
nc_s2 %>% 
  mutate(geometry = st_as_sfc(s2_as_binary(geometry))) %>% 
  st_as_sf()
#> Simple feature collection with 100 features and 1 field
#> geometry type:  GEOMETRY
#> dimension:      XY
#> bbox:           xmin: -84.32385 ymin: 33.88199 xmax: -75.45698 ymax: 36.58965
#> CRS:            NA
#> # A tibble: 100 x 2
#>    NAME                                                                 geometry
#>    <chr>                                                              <GEOMETRY>
#>  1 Ashe       POLYGON ((-81.45289 36.23959, -81.43104 36.26072, -81.41233 36.26…
#>  2 Alleghany  POLYGON ((-81.17667 36.41544, -81.15337 36.42474, -81.1384 36.417…
#>  3 Surry      POLYGON ((-80.45301 36.25709, -80.43531 36.55104, -80.61105 36.55…
#>  4 Currituck  MULTIPOLYGON (((-75.94193 36.29434, -75.95751 36.25945, -75.91376…
#>  5 Northampt… POLYGON ((-77.14196 36.41706, -77.13932 36.45648, -77.12733 36.47…
#>  6 Hertford   POLYGON ((-76.7075 36.26613, -76.74135 36.31517, -76.92408 36.392…
#>  7 Camden     POLYGON ((-76.01735 36.33773, -76.03288 36.33598, -76.04395 36.35…
#>  8 Gates      POLYGON ((-76.46035 36.3739, -76.50246 36.45229, -76.49834 36.503…
#>  9 Warren     POLYGON ((-78.13472 36.23658, -78.10963 36.21351, -78.05835 36.21…
#> 10 Stokes     POLYGON ((-80.02406 36.54502, -80.0481 36.54713, -80.43531 36.551…
#> # … with 90 more rows
```

## Acknowledgment

This project gratefully acknowledges financial
[support](https://www.r-consortium.org/projects) from the

<a href="https://www.r-consortium.org/projects/awarded-projects">
<img src="http://pebesma.staff.ifgi.de/RConsortium_Horizontal_Pantone.png" width="300">
</a>
