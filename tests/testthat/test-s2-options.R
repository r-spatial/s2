
test_that("s2_options() works", {
  expect_s3_class(s2_options(), "s2_options")
})

test_that("s2_options() errors are readable", {
  expect_error(s2_intersects("POINT EMPTY", "POINT EMPTY", options = NULL), "must be created using")
  expect_error(s2_options(model = "not a model"), "must be one of")
  expect_error(s2_options(snap_radius = 100), "radius is too large")
  expect_error(s2_snap_level(31), "between 1 and 30")
})
