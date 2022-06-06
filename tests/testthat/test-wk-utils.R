
test_that("the projection/unprojection filter errors for invalid input", {
  expect_error(s2_unprojection_filter(NULL), "must be a ")
  expect_error(s2_unprojection_filter(wk::wk_void_handler(), projection = NULL), "must be an")
  expect_error(s2_unprojection_filter(wk::wk_void_handler(), tessellate_tol = -1), "must be")
  expect_error(s2_unprojection_filter(wk::wk_void_handler(), tessellate_tol = -1), "must be")
})

test_that("unprojection using a wk filter works", {
  expect_equal(
    wk::wk_handle(wk::xy(0, 0), s2_unprojection_filter(wk::xy_writer())),
    wk::xyz(1, 0, 0)
  )

  expect_identical(
    wk::wk_handle(
      wk::wkt("LINESTRING (0 0, 0 45, -60 45)"),
      s2_unprojection_filter(wk::wkt_format_handler(precision = 4))
    ),
    # 0.7071 ~ sqrt(2) / 2
    "LINESTRING Z (1 0 0, 0.7071 0 0.7071, 0.3536 -0.6124 0.7071)"
  )

  expect_identical(
    wk::wk_handle(
      wk::wkt("POLYGON ((0 0, 0 45, -60 45, 0 0))"),
      s2_unprojection_filter(wk::wkt_format_handler(precision = 4))
    ),
    # 0.7071 ~ sqrt(2) / 2
    "POLYGON Z ((1 0 0, 0.7071 0 0.7071, 0.3536 -0.6124 0.7071, 1 0 0))"
  )

  expect_identical(
    wk::wk_handle(wk::wkt(NA_character_), s2_unprojection_filter(wk::wkt_writer())),
    wk::wkt(NA_character_)
  )
})

test_that("tessellating + unprojection using a wk filter works", {
  # using big examples here, so use a tolerance of 100 km (forces
  # adding at least one point)
  tol <- 100000 / s2_earth_radius_meters()

  expect_equal(
    wk::wk_handle(
      wk::xy(0, 0),
      s2_unprojection_filter(wk::xy_writer(), tessellate_tol = tol)
    ),
    wk::xyz(1, 0, 0)
  )

  expect_identical(
    wk::wk_handle(
      wk::wkt("LINESTRING (0 0, 0 45, -60 45)"),
      s2_unprojection_filter(wk::wkb_writer(), tessellate_tol = tol)
    ) %>%
      wk::wk_set_geodesic(TRUE) %>%
      s2_num_points(),
    6L
  )

  expect_identical(
    wk::wk_handle(
      wk::wkt("POLYGON ((0 0, 0 45, -60 45, 0 0))"),
      s2_unprojection_filter(wk::wkb_writer(), tessellate_tol = tol)
    ) %>%
      wk::wk_set_geodesic(TRUE) %>%
      s2_num_points(),
    8L
  )

  expect_identical(
    wk::wk_handle(wk::wkt(NA_character_), s2_unprojection_filter(wk::wkt_writer(), tessellate_tol = tol)),
    wk::wkt(NA_character_)
  )
})

test_that("projection using a wk filter works", {
  expect_equal(
    wk::wk_handle(wk::xyz(1, 0, 0), s2_projection_filter(wk::xy_writer())),
    wk::xy(0, 0)
  )

  expect_identical(
    wk::wk_handle(
      wk::wkt("LINESTRING Z (1 0 0, 0.7071 0 0.7071, 0.3536 -0.6124 0.7071)"),
      s2_projection_filter(wk::wkt_format_handler(precision = 4))
    ),
    "LINESTRING (0 0, 0 45, -60 45)"
  )

  expect_identical(
    wk::wk_handle(
      wk::wkt("POLYGON Z ((1 0 0, 0.7071 0 0.7071, 0.3536 -0.6124 0.7071, 1 0 0))"),
      s2_projection_filter(wk::wkt_format_handler(precision = 4))
    ),
    "POLYGON ((0 0, 0 45, -60 45, 0 0))"
  )

  expect_identical(
    wk::wk_handle(wk::wkt(NA_character_), s2_projection_filter(wk::wkt_writer())),
    wk::wkt(NA_character_)
  )
})

test_that("projection + tessellating using a wk filter works", {
  tol <- 100000 / s2_earth_radius_meters()

  expect_equal(
    wk::wk_handle(wk::xyz(1, 0, 0), s2_projection_filter(wk::xy_writer(), tessellate_tol = tol)),
    wk::xy(0, 0)
  )

  expect_identical(
    wk::wk_handle(
      wk::wkt("LINESTRING Z (1 0 0, 0.7071 0 0.7071, 0.3536 -0.6124 0.7071)"),
      s2_projection_filter(wk::wkb_writer(), tessellate_tol = tol)
    ) %>%
      wk::wk_set_geodesic(TRUE) %>%
      s2_num_points(),
    6L
  )

  expect_identical(
    wk::wk_handle(
      wk::wkt("POLYGON Z ((1 0 0, 0.7071 0 0.7071, 0.3536 -0.6124 0.7071, 1 0 0))"),
      s2_projection_filter(wk::wkb_writer(), tessellate_tol = tol)
    ) %>%
      wk::wk_set_geodesic(TRUE) %>%
      s2_num_points(),
    8L
  )

  expect_identical(
    wk::wk_handle(wk::wkt(NA_character_), s2_projection_filter(wk::wkt_writer(), tessellate_tol = tol)),
    wk::wkt(NA_character_)
  )
})

test_that("errors are propagated through the coordinate filter", {
  expect_error(
    wk::wk_handle(wk::new_wk_wkt("POINT ENTPY"),  s2_projection_filter(wk::wk_void_handler())),
    "ENTPY"
  )
})

test_that("early returns are respected by s2_projection_filter()", {
  big_line <- as_wkt(s2::s2_make_line(1:10, 1:10))
  unprojected <- wk::wk_handle(big_line, s2_unprojection_filter(wk::wkt_writer()))
  expect_identical(
    wk::wk_handle(unprojected, s2_projection_filter(wk::wkt_format_handler(max_coords = 2))),
    "LINESTRING (1 1, 2 2...",
  )
  expect_identical(
    wk::wk_handle(
      unprojected,
      s2_projection_filter(wk::wkt_format_handler(max_coords = 2), tessellate_tol = 1e-8)
    ),
    "LINESTRING (1 1, 1.062476 1.062512...",
  )
})

test_that("early returns are respected by s2_unprojection_filter()", {
  big_line <- as_wkt(s2::s2_make_line(1:10, 1:10))
  expect_identical(
    wk::wk_handle(big_line, s2_unprojection_filter(wk::wkt_format_handler(max_coords = 2))),
    "LINESTRING Z (0.9996954 0.01744975 0.01745241, 0.998782 0.03487824 0.0348995...",
  )
  expect_identical(
    wk::wk_handle(
      big_line,
      s2_unprojection_filter(wk::wkt_format_handler(max_coords = 2), tessellate_tol = 1e-8)
    ),
    "LINESTRING Z (0.9996954 0.01744975 0.01745241, 0.9996562 0.01853987 0.01854306...",
  )
})

test_that("mercator projection works", {
  expect_equal(
    wk::wk_handle(
      wk::xy(c(0, 180), 0),
      s2_unprojection_filter(s2_projection_filter(wk::xy_writer(), s2_projection_mercator()))
    ),
    wk::xy(c(0, 20037508), 0)
  )
})

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
