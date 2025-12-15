# Low-level wk filters and handlers

Low-level wk filters and handlers

## Usage

``` r
# S3 method for class 's2_geography'
wk_handle(
  handleable,
  handler,
  ...,
  s2_projection = s2_projection_plate_carree(),
  s2_tessellate_tol = Inf
)

s2_geography_writer(
  oriented = FALSE,
  check = TRUE,
  projection = s2_projection_plate_carree(),
  tessellate_tol = Inf,
  use_altrep = !isTRUE(getOption("s2.disable_altrep"))
)

# S3 method for class 's2_geography'
wk_writer(handleable, ...)

s2_trans_point()

s2_trans_lnglat()

s2_projection_plate_carree(x_scale = 180)

s2_projection_mercator(x_scale = 20037508.3427892)

s2_hemisphere(centre)

s2_world_plate_carree(epsilon_east_west = 0, epsilon_north_south = 0)

s2_projection_orthographic(centre = s2_lnglat(0, 0))
```

## Arguments

- handleable:

  A geometry vector (e.g.,
  [`wkb()`](https://paleolimbot.github.io/wk/reference/wkb.html),
  [`wkt()`](https://paleolimbot.github.io/wk/reference/wkt.html),
  [`xy()`](https://paleolimbot.github.io/wk/reference/xy.html),
  [`rct()`](https://paleolimbot.github.io/wk/reference/rct.html), or
  `sf::st_sfc()`) for which
  [`wk_handle()`](https://paleolimbot.github.io/wk/reference/wk_handle.html)
  is defined.

- handler:

  A
  [wk_handler](https://paleolimbot.github.io/wk/reference/wk_handle.html)
  object.

- ...:

  Passed to the
  [`wk_handle()`](https://paleolimbot.github.io/wk/reference/wk_handle.html)
  method.

- oriented:

  TRUE if polygon ring directions are known to be correct (i.e.,
  exterior rings are defined counter clockwise and interior rings are
  defined clockwise).

- check:

  Use `check = FALSE` to skip error on invalid geometries

- projection, s2_projection:

  One of `s2_projection_plate_carree()` or `s2_projection_mercator()`

- tessellate_tol, s2_tessellate_tol:

  An angle in radians. Points will not be added if a line segment is
  within this distance of a point.

- use_altrep:

  A flag indicating whether ALTREP representation of s2 geography
  vectors should be used with support for data serialization (default:
  `TRUE` on R 4.3.0 and later, set the option `s2.disable_altrep` to
  disable)

- x_scale:

  The maximum x value of the projection

- centre:

  The center point of the orthographic projection

- epsilon_east_west, epsilon_north_south:

  Use a positive number to define the edges of a Cartesian world
  slightly inward from -180, -90, 180, 90. This may be used to define a
  world outline for a projection where projecting at the extreme edges
  of the earth results in a non-finite value.

## Value

- `s2_projection_plate_carree()`, `s2_projection_mercator()`: An
  external pointer to an S2 projection.
