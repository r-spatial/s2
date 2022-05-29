
test_that("s2_data_country() works", {
  expect_s3_class(s2_data_countries("Germany"), "s2_geography")
  expect_length(s2_data_countries("Germany"), 1)

  expect_s3_class(s2_data_countries("Europe"), "s2_geography")
  expect_length(s2_data_countries("Europe"), 39)

  expect_s3_class(s2_data_countries(), "s2_geography")
  expect_length(s2_data_countries(), 177)
})

test_that("s2_data_timezone() works", {
  expect_s3_class(s2_data_timezones(), "s2_geography")
  expect_length(s2_data_timezones(), 120)

  expect_s3_class(s2_data_timezones(-4), "s2_geography")
  expect_length(s2_data_timezones(-4), 3)

  expect_s3_class(s2_data_timezones(-15, 15), "s2_geography")
  expect_length(s2_data_timezones(-15, 15), 120)
})

test_that("s2_data_cities() works", {
  expect_s3_class(s2_data_cities(), "s2_geography")
  expect_length(s2_data_cities(), 243)

  expect_s3_class(s2_data_cities("Cairo"), "s2_geography")
  expect_length(s2_data_cities("Cairo"), 1)
})
