
test_that("wk_handle() for s2_geography works", {
  for (name in names(s2_data_example_wkt)) {
    geog <- wk::wk_handle(
      s2_data_example_wkt[[name]],
      s2_geography_writer()
    )

    geog2 <- wk::wk_handle(
      geog,
      s2_geography_writer(check = TRUE, oriented = TRUE)
    )

    expect_equal(wk::wk_coords(geog), wk::wk_coords(geog2))
  }
})

test_that("the s2_geography_writer() works", {
  # nc has some rings that get reordered by this operation
  for (name in setdiff(names(s2_data_example_wkt), "nc")) {
    geog <- wk::wk_handle(
      s2_data_example_wkt[[name]],
      s2_geography_writer()
    )

    expect_equal(
      wk::wk_coords(as_wkt(geog))[c("x", "y")],
      wk::wk_coords(s2_data_example_wkt[[name]])[c("x", "y")]
    )
  }
})

test_that("the s2_trans_point() and s2_trans_lnglat() work", {
  lng_lats <- s2_lnglat(-179:179, 45)
  points <- as_s2_point(lng_lats)
  expect_identical(as_s2_lnglat(lng_lats), lng_lats)

  expect_equal(
    wk::wk_transform(lng_lats, s2_trans_point()),
    wk::wk_set_crs(points, NULL)
  )

  expect_equal(
    wk::wk_transform(points, s2_trans_lnglat()),
    wk::wk_set_crs(lng_lats, NULL)
  )
})

test_that("s2_geography_writer() with tesselate_tol works", {
  # using big examples here, so use a tolerance of 100 km (forces
  # adding at least one point)
  tol <- 100000 / s2_earth_radius_meters()

  expect_equal(
    wk::as_xy(
      wk::wk_handle(
        wk::xy(0, 0),
        s2_geography_writer(tessellate_tol = tol)
      )
    ),
    wk::xy(0, 0, crs = wk::wk_crs_longlat())
  )

  expect_identical(
    wk::wk_handle(
      wk::wkt("LINESTRING (0 0, 0 45, -60 45)"),
      s2_geography_writer(tessellate_tol = tol)
    ) %>%
      s2_num_points(),
    6L
  )

  expect_identical(
    wk::wk_handle(
      wk::wkt("POLYGON ((0 0, 0 45, -60 45, 0 0))"),
      s2_geography_writer(tessellate_tol = tol)
    ) %>%
      s2_num_points(),
    8L
  )
})

test_that("s2_geography_writer() with tesselate_tol works with real data", {
  tol <- 1000 / s2_earth_radius_meters()

  countries_tes <- wk::wk_handle(
    s2::s2_data_tbl_countries$geometry,
    s2_geography_writer(tessellate_tol = tol)
  )

  expect_true(
    sum(s2_num_points(countries_tes)) >
      sum(s2_num_points(s2_data_countries()))
  )
})

test_that("wk_handle + tessellate_tol works", {
  tol <- 100000 / s2_earth_radius_meters()

  expect_equal(
    wk::wk_handle(
      as_s2_geography(s2_lnglat(0, 0)),
      wk::xy_writer(),
      s2_tessellate_tol = tol
    ),
    wk::xy(0, 0)
  )

  expect_identical(
    wk::wk_handle(
      as_s2_geography("LINESTRING (0 0, 0 45, -60 45)"),
      s2_geography_writer(),
      s2_tessellate_tol = tol
    ) %>%
      s2_num_points(),
    6L
  )

  expect_identical(
    wk::wk_handle(
      as_s2_geography("POLYGON ((0 0, 0 45, -60 45, 0 0))"),
      s2_geography_writer(),
      s2_tessellate_tol = tol
    ) %>%
      s2_num_points(),
    8L
  )
})

test_that("s2_geography_writer() with tesselate_tol works with real data", {
  tol <- 1000 / s2_earth_radius_meters()

  countries <- s2_data_countries()
  countries_tes <- wk::wk_handle(
    countries,
    s2_geography_writer(),
    s2_tessellate_tol = tol
  )

  expect_true(
    sum(s2_num_points(countries_tes)) >
      sum(s2_num_points(s2_data_countries()))
  )
})
