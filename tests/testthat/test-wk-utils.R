
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

test_that("wk_handle() for s2_geography works for s2_point projection", {
  for (name in names(s2_data_example_wkt)) {
    geog <- wk::wk_handle(
      s2_data_example_wkt[[name]],
      s2_geography_writer()
    )

    geog2 <- wk::wk_handle(
      geog,
      s2_geography_writer(
        check = TRUE,
        oriented = TRUE,
        projection = NULL
      ),
      s2_projection = NULL
    )

    expect_identical(wk::wk_coords(geog), wk::wk_coords(geog2))
  }
})

test_that("wk_writer() works for s2_geography()", {
  expect_s3_class(wk::wk_writer(s2_geography()), "s2_geography_writer")
})

test_that("the s2_geography_writer() works for example WKT", {
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

test_that("wk_handle() works for example WKT", {
  for (name in names(s2_data_example_wkt)) {
    geog <- wk::wk_handle(
      s2_data_example_wkt[[name]],
      s2_geography_writer()
    )

    expect_wkt_equal(
      wk_handle(geog, s2_geography_writer()),
      geog,
      precision = 14
    )
  }
})

test_that("wk_handle() works for example WKT with tessellation", {
  for (name in names(s2_data_example_wkt)) {
    geog <- wk::wk_handle(
      s2_data_example_wkt[[name]],
      s2_geography_writer()
    )

    expect_wkt_equal(
      # use a big but non-infinite number to trigger the tessellator
      wk_handle(geog, s2_geography_writer(), s2_tessellate_tol = 1e10),
      geog,
      precision = 14
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
    s2_geography_writer(check = FALSE),
    s2_tessellate_tol = tol
  )

  expect_true(
    sum(s2_num_points(countries_tes)) >
      sum(s2_num_points(s2_data_countries()))
  )
})

test_that("wk_handle() for s2_geography works with s2_projection_mercator()", {
  # sf::sf_project("EPSG:4326", "EPSG:3857", wk::xy(30, 10)) %>% dput()
  geog <- wk::wk_handle(
    wk::xy(3339584.72379821, 1118889.97485796),
    s2_geography_writer(projection = s2_projection_mercator())
  )

  expect_equal(
    wk::wk_handle(
      geog,
      wk::xy_writer(),
      s2_projection = s2_projection_mercator()
    ),
    wk::xy(3339584.72379821, 1118889.97485796)
  )

  expect_equal(
    wk::wk_handle(
      geog,
      wk::xy_writer(),
      s2_projection = s2_projection_mercator(),
      s2_tessellate_tol = 1e10
    ),
    wk::xy(3339584.72379821, 1118889.97485796)
  )
})

test_that("s2_geography_writer() works with s2_projection_mercator()", {
  # sf::sf_project("EPSG:4326", "EPSG:3857", wk::xy(30, 10)) %>% dput()
  expect_equal(
    wk::as_xy(
      wk::wk_handle(
        wk::xy(3339584.72379821, 1118889.97485796),
        s2_geography_writer(projection = s2_projection_mercator())
      )
    ),
    wk::xy(30, 10, crs = wk::wk_crs_longlat())
  )
})

test_that("wk_handle() for s2_geography works with s2_projection_orthographic()", {
  geog <- as_s2_geography(c("POINT (0 0)", "POINT (0 45)", "POINT (45 0)"))
  result <- wk::wk_handle(
    geog,
    wk::xy_writer(),
    s2_projection = s2_projection_orthographic()
  )

  expect_equal(
    result,
    wk::xy(
      c(0, 0, sqrt(2) / 2),
      c(0, sqrt(2) / 2, 0)
    )
  )
})

test_that("orthographic projection maintains 'north up' orientation", {
  result_coords <- wk::wk_coords(
    as_s2_geography(s2_lnglat(-64, c(45, 50))),
    s2_projection = s2_projection_orthographic(s2_lnglat(-64, 45))
  )

  expect_equal(result_coords$x[1], 0)
  expect_equal(result_coords$y[1], 0)
  expect_equal(result_coords$x[1], result_coords$x[2])

  # proj_result <- sf::sf_project(
  #   "EPSG:4326",
  #   "+proj=ortho +lon_0=-64 +lat_0=45 +ellips=sphere",
  #   s2::s2_lnglat(-64, c(45, 50))
  # ) / 6370997
  # result_coords <- wk::wk_coords(wk::as_xy(proj_result))
})

test_that("s2_geography_writer() works with s2_projection_mercator()", {
  # sf::sf_project("EPSG:4326", "EPSG:3857", wk::xy(30, 10)) %>% dput()
  xy <- wk::xy(
    c(0, 0, sqrt(2) / 2),
    c(0, sqrt(2) / 2, 0)
  )

  geog <- wk::wk_handle(
    xy,
    s2_geography_writer(projection = s2_projection_orthographic())
  )
  expect_identical(
    s2_as_text(geog, precision = 5),
    c("POINT (0 0)", "POINT (0 45)", "POINT (45 0)")
  )
})

test_that("s2_hemisphere() works", {
  expect_equal(
    s2_area(s2_hemisphere(s2_lnglat(0, 0)), radius = 1),
    2 * pi
  )
})

test_that("s2_world_plate_carree() works", {
  world0 <- s2_world_plate_carree(0, 0)
  expect_identical(
    wk::wk_bbox(wk::wkt(s2_as_text(world0))),
    wk::rct(-180, -90, 180, 90)
  )

  world_eps <- s2_world_plate_carree(1, 2)
  expect_identical(
    wk::wk_bbox(wk::wkt(s2_as_text(world_eps))),
    wk::rct(-179, -88, 179, 88)
  )
})
