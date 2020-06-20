
test_that("s2_closest_feature() works", {
  cities <- s2_data_cities("London")
  countries <- s2_data_countries()

  # with zero length y, results will all be empty
  expect_identical(s2_closest_feature(cities, character(0)), rep_len(NA_integer_, length(cities)))

  # should correctly identify that London is closest to United Kingdom
  country_match <- s2_closest_feature(cities, countries)
  expect_identical(s2_data_tbl_countries$name[country_match], "United Kingdom")
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
