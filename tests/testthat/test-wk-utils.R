
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
    ) %>% s2_num_points(),
    6L
  )

  expect_identical(
    wk::wk_handle(
      wk::wkt("POLYGON ((0 0, 0 45, -60 45, 0 0))"),
      s2_unprojection_filter(wk::wkb_writer(), tessellate_tol = tol)
    ) %>% s2_num_points(),
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
    ) %>% s2_num_points(),
    6L
  )

  expect_identical(
    wk::wk_handle(
      wk::wkt("POLYGON Z ((1 0 0, 0.7071 0 0.7071, 0.3536 -0.6124 0.7071, 1 0 0))"),
      s2_projection_filter(wk::wkb_writer(), tessellate_tol = tol)
    ) %>% s2_num_points(),
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
