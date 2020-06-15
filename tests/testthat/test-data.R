
test_that("s2data_country() works", {
  expect_is(s2data_countries("Germany"), "s2_geography")
  expect_length(s2data_countries("Germany"), 1)

  expect_is(s2data_countries("Europe"), "s2_geography")
  expect_length(s2data_countries("Europe"), 39)

  expect_is(s2data_countries(), "s2_geography")
  expect_length(s2data_countries(), 177)
})

test_that("s2data_timezone() works", {
  expect_is(s2data_timezones(), "s2_geography")
  expect_length(s2data_timezones(), 120)

  expect_is(s2data_timezones(-4), "s2_geography")
  expect_length(s2data_timezones(-4), 3)

  expect_is(s2data_timezones(-15, 15), "s2_geography")
  expect_length(s2data_timezones(-15, 15), 120)
})

test_that("s2data_cities() works", {
  expect_is(s2data_cities(), "s2_geography")
  expect_length(s2data_cities(), 243)

  expect_is(s2data_cities("Cairo"), "s2_geography")
  expect_length(s2data_cities("Cairo"), 1)
})
