
test_that("s2_closest_feature() works", {
  cities <- s2_data_cities("London")
  countries <- s2_data_countries()

  # with zero length y, results will all be empty
  expect_identical(s2_closest_feature(cities, character(0)), rep_len(NA_integer_, length(cities)))

  # should correctly identify that London is closest to United Kingdom
  country_match <- s2_closest_feature(cities, countries)
  expect_identical(s2_data_tbl_countries$name[country_match], "United Kingdom")
})
