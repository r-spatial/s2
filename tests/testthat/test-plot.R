
test_that("s2_plot works", {
  skip_if_not_installed("vctrs")

  x <- s2_data_countries()
  expect_identical(s2_plot(x), x)
  s2_plot(s2_data_cities(), add = TRUE)

  expect_identical(s2_plot(x, simplify = FALSE), x)
  expect_identical(s2_plot(x, plot_hemisphere = TRUE), x)
  expect_identical(s2_plot(x, centre = s2_lnglat(0, 0)), x)
})

test_that("s2_plot works for all examples", {
  skip_if_not_installed("vctrs")

  for (name in names(s2_data_example_wkt)) {
    geog <- as_s2_geography(s2_data_example_wkt[[name]])

    # need non-nulls for now:
    # https://github.com/paleolimbot/wk/issues/143
    geog <- geog[!is.na(geog)]

    expect_identical(s2_plot(geog), geog)
  }
})

test_that("plot() works for vector classes", {
  skip_if_not_installed("vctrs")

  x <- as_s2_geography("POINT (0 1)")
  expect_identical(plot(x), x)

  x <- s2_covering_cell_ids(as_s2_geography("POLYGON ((0 0, 1 0, 0 1, 0 0))"))
  expect_identical(plot(x), x)

  x <- as_s2_cell(as_s2_geography("POINT (0 1)"))
  expect_identical(plot(x), x)

  x <- s2_cell_parent(x, 7)
  expect_identical(plot(x), x)
})
