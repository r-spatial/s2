# S2 cell operators

S2 cell operators

## Usage

``` r
s2_cell_is_valid(x)

s2_cell_debug_string(x)

s2_cell_to_lnglat(x)

s2_cell_center(x)

s2_cell_boundary(x)

s2_cell_polygon(x)

s2_cell_vertex(x, k)

s2_cell_level(x)

s2_cell_is_leaf(x)

s2_cell_is_face(x)

s2_cell_area(x, radius = s2_earth_radius_meters())

s2_cell_area_approx(x, radius = s2_earth_radius_meters())

s2_cell_parent(x, level = -1L)

s2_cell_child(x, k)

s2_cell_edge_neighbour(x, k)

s2_cell_contains(x, y)

s2_cell_distance(x, y, radius = s2_earth_radius_meters())

s2_cell_max_distance(x, y, radius = s2_earth_radius_meters())

s2_cell_may_intersect(x, y)

s2_cell_common_ancestor_level(x, y)

s2_cell_common_ancestor_level_agg(x, na.rm = FALSE)
```

## Arguments

- x, y:

  An [`s2_cell()`](https://r-spatial.github.io/s2/reference/s2_cell.md)
  vector

- k:

  An integer between 0 and 3

- radius:

  The radius to use (e.g.,
  [`s2_earth_radius_meters()`](https://r-spatial.github.io/s2/reference/s2_earth_radius_meters.md))

- level:

  An integer between 0 and 30, inclusive.

- na.rm:

  Remove NAs prior to computing aggregate?
