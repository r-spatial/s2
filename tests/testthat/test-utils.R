
test_that("recycle_common works", {
  expect_identical(recycle_common(1, 2), list(1, 2))
  expect_identical(recycle_common(1, b = 2), list(1, b = 2))
  expect_identical(recycle_common(1, 2:4), list(c(1, 1, 1), c(2L, 3L, 4L)))
  expect_identical(recycle_common(numeric(0), 2), list(numeric(0), numeric(0)))
  expect_error(recycle_common(numeric(0), 2:4), "Incompatible lengths")
})

test_that("wkt tester works", {
  expect_wkt_equal("POINT (0.123456 0)", "POINT (0.1234561 0)", precision = 6)
  expect_failure(expect_wkt_equal("POINT (0.123456 0)", "POINT (0.1234561 0)", precision = 7))
})

test_that("almost equal expectation works", {
  expect_near(0.001, 0, epsilon = 0.0011)
  expect_failure(expect_near(0.001, 0, epsilon = 0.0009))
})
