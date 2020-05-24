
test_that("s2earth_radius_meters works", {
  expect_is(s2earth_radius_meters(), "numeric")
  expect_length(s2earth_radius_meters(), 1)
})
