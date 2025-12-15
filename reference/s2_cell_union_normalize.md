# S2 cell union operators

S2 cell union operators

## Usage

``` r
s2_cell_union_normalize(x)

s2_cell_union_contains(x, y)

s2_cell_union_intersects(x, y)

s2_cell_union_intersection(x, y)

s2_cell_union_union(x, y)

s2_cell_union_difference(x, y)

s2_covering_cell_ids(
  x,
  min_level = 0,
  max_level = 30,
  max_cells = 8,
  buffer = 0,
  interior = FALSE,
  radius = s2_earth_radius_meters()
)

s2_covering_cell_ids_agg(
  x,
  min_level = 0,
  max_level = 30,
  max_cells = 8,
  buffer = 0,
  interior = FALSE,
  radius = s2_earth_radius_meters(),
  na.rm = FALSE
)
```

## Arguments

- x, y:

  An
  [s2_geography](https://r-spatial.github.io/s2/reference/as_s2_geography.md)
  or
  [`s2_cell_union()`](https://r-spatial.github.io/s2/reference/s2_cell_union.md).

- min_level, max_level:

  The minimum and maximum levels to constrain the covering.

- max_cells:

  The maximum number of cells in the covering. Defaults to 8.

- buffer:

  A distance to buffer outside the geography

- interior:

  Use `TRUE` to force the covering inside the geography.

- radius:

  The radius to use (e.g.,
  [`s2_earth_radius_meters()`](https://r-spatial.github.io/s2/reference/s2_earth_radius_meters.md))

- na.rm:

  Remove NAs prior to computing aggregate?
