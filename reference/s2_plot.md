# Plot S2 Geographies

Plot S2 Geographies

## Usage

``` r
s2_plot(
  x,
  ...,
  asp = 1,
  xlab = "",
  ylab = "",
  rule = "evenodd",
  add = FALSE,
  plot_hemisphere = FALSE,
  simplify = TRUE,
  centre = NULL
)
```

## Arguments

- x:

  A [`wkb()`](https://paleolimbot.github.io/wk/reference/wkb.html) or
  [`wkt()`](https://paleolimbot.github.io/wk/reference/wkt.html)

- ...:

  Passed to plotting functions for features:
  [`graphics::points()`](https://rdrr.io/r/graphics/points.html) for
  point and multipoint geometries,
  [`graphics::lines()`](https://rdrr.io/r/graphics/lines.html) for
  linestring and multilinestring geometries, and
  [`graphics::polypath()`](https://rdrr.io/r/graphics/polypath.html) for
  polygon and multipolygon geometries.

- asp, xlab, ylab:

  Passed to
  [`graphics::plot()`](https://rdrr.io/r/graphics/plot.default.html)

- rule:

  The rule to use for filling polygons (see
  [`graphics::polypath()`](https://rdrr.io/r/graphics/polypath.html))

- add:

  Should a new plot be created, or should `handleable` be added to the
  existing plot?

- plot_hemisphere:

  Plot the outline of the earth

- simplify:

  Use `FALSE` to skip the simplification step

- centre:

  The longitude/latitude point of the centre of the orthographic
  projection

## Value

The input, invisibly

## Examples

``` r
s2_plot(s2_data_countries())
s2_plot(s2_data_cities(), add = TRUE)

```
