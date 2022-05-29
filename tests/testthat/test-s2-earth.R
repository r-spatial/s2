
test_that("s2_earth_radius_meters works", {
  expect_type(s2_earth_radius_meters(), "double")
  expect_length(s2_earth_radius_meters(), 1)
})
