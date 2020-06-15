
test_that("s2_earth_radius_meters works", {
  expect_is(s2_earth_radius_meters(), "numeric")
  expect_length(s2_earth_radius_meters(), 1)
})
