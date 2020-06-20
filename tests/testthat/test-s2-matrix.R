
test_that("s2_closest|farthest_feature() works", {
  cities <- s2_data_cities("London")
  countries <- s2_data_countries()

  # with zero length y, results will all be empty
  expect_identical(s2_closest_feature(cities, character(0)), rep_len(NA_integer_, length(cities)))

  # should correctly identify that London is closest to United Kingdom
  country_match <- s2_closest_feature(cities, countries)
  expect_identical(s2_data_tbl_countries$name[country_match], "United Kingdom")

  country_match_farthest <- s2_farthest_feature(cities, countries)
  expect_identical(s2_data_tbl_countries$name[country_match_farthest], "New Zealand")
})

test_that("matrix predicates work", {
  expect_identical(
    s2_contains_matrix(
      "POLYGON ((0 0, 1 0, 1 1, 0 1, 0 0))",
      c("POINT (-1 0.5)", "POINT (0.5 0.5)", "POINT (2 0.5)"),
    ),
    list(2L)
  )

  expect_identical(
    s2_within_matrix(
      c("POINT (-1 0.5)", "POINT (0.5 0.5)", "POINT (2 0.5)"),
      "POLYGON ((0 0, 1 0, 1 1, 0 1, 0 0))"
    ),
    list(integer(0), 1L, integer(0))
  )

  expect_identical(
    s2_covers_matrix(
      "POLYGON ((0 0, 1 0, 1 1, 0 1, 0 0))",
      c("POINT (-1 0.5)", "POINT (0.5 0.5)", "POINT (2 0.5)"),
    ),
    list(2L)
  )

  expect_identical(
    s2_covered_by_matrix(
      c("POINT (-1 0.5)", "POINT (0.5 0.5)", "POINT (2 0.5)"),
      "POLYGON ((0 0, 1 0, 1 1, 0 1, 0 0))"
    ),
    list(integer(0), 1L, integer(0))
  )

  expect_identical(
    s2_intersects_matrix(
      c("POINT (-1 0.5)", "POINT (0.5 0.5)", "POINT (2 0.5)"),
      "POLYGON ((0 0, 1 0, 1 1, 0 1, 0 0))"
    ),
    list(integer(0), 1L, integer(0))
  )

  expect_identical(
    s2_disjoint_matrix(
      c("POINT (-1 0.5)", "POINT (0.5 0.5)", "POINT (2 0.5)"),
      "POLYGON ((0 0, 1 0, 1 1, 0 1, 0 0))"
    ),
    list(1L, integer(0), 1L)
  )

  expect_identical(
    s2_equals_matrix(
      c("POINT (-1 0.5)", "POINT (0.5 0.5)", "POINT (2 0.5)",
        "POLYGON ((1 0, 1 1, 0 1, 0 0, 1 0))"),
      "POLYGON ((0 0, 1 0, 1 1, 0 1, 0 0))"
    ),
    list(integer(0), integer(0), integer(0), 1L)
  )

  expect_identical(
    s2_touches_matrix(
      c("POINT (0.5 0.5)", "POINT (0 0)", "POINT  (-0.5 -0.5)"),
      "POLYGON ((0 0, 0 1, 1 1, 0 0))"
    ),
    list(integer(0), 1L, integer(0))
  )

  expect_identical(
    s2_dwithin_matrix(
      c("POINT (-1 0.5)", "POINT (0.5 0.5)", "POINT (2 0.5)"),
      "POLYGON ((0 0, 1 0, 1 1, 0 1, 0 0))",
      0
    ),
    list(integer(0), 1L, integer(0))
  )
  expect_identical(
    s2_dwithin_matrix(
      c("POINT (-1 0.5)", "POINT (0.5 0.5)", "POINT (2 0.5)"),
      "POLYGON ((0 0, 1 0, 1 1, 0 1, 0 0))",
      s2_earth_radius_meters()
    ),
    list(1L, 1L, 1L)
  )
})

test_that("s2_(max_)?distance_matrix() works", {
  x <- c("POINT (0 0)", "POINT (0 90)")
  y <- c("POINT (180 0)", "POINT (0 -90)", "POINT (0 0)")

  expect_equal(
    s2_distance_matrix(x, x, radius = 180 / pi),
    matrix(c(0, 90, 90, 0), ncol = 2)
  )
  expect_equal(
    s2_distance_matrix(x, y, radius = 180 / pi),
    matrix(c(180, 90, 90, 180, 0, 90), ncol = 3)
  )

  # max distance is the same for points
  expect_equal(
    s2_max_distance_matrix(x, x, radius = 180 / pi),
    matrix(c(0, 90, 90, 0), ncol = 2)
  )

  expect_equal(
    s2_max_distance_matrix(x, y, radius = 180 / pi),
    matrix(c(180, 90, 90, 180, 0, 90), ncol = 3)
  )

  # NA handling for both rows and cols
  y[2] <- NA
  expect_true(all(is.na(s2_distance_matrix(x, y)[, 2])))
  expect_true(all(is.na(s2_max_distance_matrix(x, y)[, 2])))

  x[2] <- NA
  expect_true(all(is.na(s2_distance_matrix(x, y)[2, ])))
  expect_true(all(is.na(s2_max_distance_matrix(x, y)[2, ])))
})
