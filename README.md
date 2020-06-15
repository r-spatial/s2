
<!-- README.md is generated from README.Rmd. Please edit that file -->

# s2

<!-- badges: start -->

[![Lifecycle:
experimental](https://img.shields.io/badge/lifecycle-experimental-orange.svg)](https://www.tidyverse.org/lifecycle/#experimental)
![R-CMD-check](https://github.com/r-spatial/s2/workflows/R-CMD-check/badge.svg)
[![codecov](https://codecov.io/gh/r-spatial/s2/branch/master/graph/badge.svg)](https://codecov.io/gh/r-spatial/s2)
<!-- badges: end -->

The goal of s2 is to provide R bindings to Google’s
[S2Geometry](https://s2geometry.io) library. The package exposes an API
similar to Google’s [BigQuery Geography
API](https://cloud.google.com/bigquery/docs/reference/standard-sql/geography_functions),
whose functions also operate on spherical geometries.

## Installation

You can install the released version of s2 from
[CRAN](https://CRAN.R-project.org) with:

``` r
install.packages("s2")
```

And the development version from [GitHub](https://github.com/) with:

``` r
# install.packages("remotes")
remotes::install_github("r-spatial/libs2")
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

The [sf package](https://r-spatial.gitub.io/sf) will soon support s2,
but until it does, you can use the [wk
package](https://paleolimbot.github.io/wk) to convert data from sf to
s2:

``` r
library(dplyr)
library(sf)

nc_s2 <- read_sf(system.file("shape/nc.shp", package = "sf")) %>% 
  mutate(geometry = as_s2_geography(wk::wkb(st_as_binary(geometry)))) %>% 
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

## Acknowledgment

This project gratefully acknowledges financial
[support](https://www.r-consortium.org/projects) from the

<a href="https://www.r-consortium.org/projects/awarded-projects">
<img src="http://pebesma.staff.ifgi.de/RConsortium_Horizontal_Pantone.png" width="300">
</a>
