# Geography Operation Options

These functions specify defaults for options used to perform operations
and construct geometries. These are used in predicates (e.g.,
[`s2_intersects()`](https://r-spatial.github.io/s2/reference/s2_contains.md)),
and boolean operations (e.g.,
[`s2_intersection()`](https://r-spatial.github.io/s2/reference/s2_boundary.md))
to specify the model for containment and how new geometries should be
constructed.

## Usage

``` r
s2_options(
  model = NULL,
  snap = s2_snap_identity(),
  snap_radius = -1,
  duplicate_edges = FALSE,
  edge_type = "directed",
  validate = FALSE,
  polyline_type = "path",
  polyline_sibling_pairs = "keep",
  simplify_edge_chains = FALSE,
  split_crossing_edges = FALSE,
  idempotent = FALSE,
  dimensions = c("point", "polyline", "polygon")
)

s2_snap_identity()

s2_snap_level(level)

s2_snap_precision(precision)

s2_snap_distance(distance)
```

## Arguments

- model:

  One of 'open', 'semi-open' (default for polygons), or 'closed'
  (default for polylines). See section 'Model'

- snap:

  Use `s2_snap_identity()`, `s2_snap_distance()`, `s2_snap_level()`, or
  `s2_snap_precision()` to specify how or if coordinate rounding should
  occur.

- snap_radius:

  As opposed to the snap function, which specifies the maximum distance
  a vertex should move, the snap radius (in radians) sets the minimum
  distance between vertices of the output that don't cause vertices to
  move more than the distance specified by the snap function. This can
  be used to simplify the result of a boolean operation. Use -1 to
  specify that any minimum distance is acceptable.

- duplicate_edges:

  Use `TRUE` to keep duplicate edges (e.g., duplicate points).

- edge_type:

  One of 'directed' (default) or 'undirected'.

- validate:

  Use `TRUE` to validate the result from the builder.

- polyline_type:

  One of 'path' (default) or 'walk'. If 'walk', polylines that backtrack
  are preserved.

- polyline_sibling_pairs:

  One of 'discard' (default) or 'keep'.

- simplify_edge_chains:

  Use `TRUE` to remove vertices that are within `snap_radius` of the
  original vertex.

- split_crossing_edges:

  Use `TRUE` to split crossing polyline edges when creating geometries.

- idempotent:

  Use `FALSE` to apply snap even if snapping is not necessary to satisfy
  vertex constraints.

- dimensions:

  A combination of 'point', 'polyline', and/or 'polygon' that can used
  to constrain the output of
  [`s2_rebuild()`](https://r-spatial.github.io/s2/reference/s2_boundary.md)
  or a boolean operation.

- level:

  A value from 0 to 30 corresponding to the cell level at which snapping
  should occur.

- precision:

  A number by which coordinates should be multiplied before being
  rounded. Rounded to the nearest exponent of 10.

- distance:

  A distance (in radians) denoting the maximum distance a vertex should
  move in the snapping process.

## Model

The geometry model indicates whether or not a geometry includes its
boundaries. Boundaries of line geometries are its end points. OPEN
geometries do not contain their boundary (`model = "open"`); CLOSED
geometries (`model = "closed"`) contain their boundary; SEMI-OPEN
geometries (`model = "semi-open"`) contain half of their boundaries,
such that when two polygons do not overlap or two lines do not cross, no
point exist that belong to more than one of the geometries. (This latter
form, half-closed, is not present in the OpenGIS "simple feature access"
(SFA) standard nor DE9-IM on which that is based). The default values
for
[`s2_contains()`](https://r-spatial.github.io/s2/reference/s2_contains.md)
(open) and covers/covered_by (closed) correspond to the SFA standard
specification of these operators.

## Examples

``` r
# use s2_options() to specify containment models, snap level
# layer creation options, and builder options
s2_options(model = "closed", snap = s2_snap_level(30))
#> $model
#> [1] 3
#> 
#> $snap
#> $level
#> [1] 30
#> 
#> attr(,"class")
#> [1] "snap_level"
#> 
#> $snap_radius
#> [1] -1
#> 
#> $duplicate_edges
#> [1] FALSE
#> 
#> $edge_type
#> [1] 1
#> 
#> $validate
#> [1] FALSE
#> 
#> $polyline_type
#> [1] 1
#> 
#> $polyline_sibling_pairs
#> [1] 2
#> 
#> $simplify_edge_chains
#> [1] FALSE
#> 
#> $split_crossing_edges
#> [1] FALSE
#> 
#> $idempotent
#> [1] FALSE
#> 
#> $dimensions
#> [1] 1 2 3
#> 
#> attr(,"class")
#> [1] "s2_options"
```
