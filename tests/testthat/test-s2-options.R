
test_that("s2_options() works", {
  expect_is(s2_options(), "s2_options")
})

test_that("s2_options() errors are readable", {
  expect_error(s2_intersects("POINT EMPTY", "POINT EMPTY", options = NULL), "must be created using")
})
