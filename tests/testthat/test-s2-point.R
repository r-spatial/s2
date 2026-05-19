
test_that("s2_point objects can be created from and converted back to R objects", {
  # in
  expect_s3_class(s2_point(1, 2, 3), "wk_xyz")
  expect_length(s2_point(1, 2, 3), 1)
  expect_s3_class(as_s2_point(matrix(c(1, 2, 3), ncol = 3)), "wk_xyz")
  point <- s2_point(1, 2, 3)
  expect_identical(as_s2_point(point), point)
  expect_identical(
    as_s2_point(s2_lnglat(0, 0)),
    s2_point(1, 0, 0)
  )
})

test_that("s2_point objects propagate NAs on convert to/from lnglat", {
  expect_identical(as_s2_point(s2_lnglat(NA, NA)), s2_point(NA, NA, NA))
  expect_identical(as_s2_point(s2_lnglat(NA, 1)), s2_point(NA, NA, NA))
  expect_identical(as_s2_point(s2_lnglat(1, NA)), s2_point(NA, NA, NA))

  expect_identical(as_s2_lnglat(s2_point(NA, NA, NA)), s2_lnglat(NA, NA))
  expect_identical(as_s2_lnglat(s2_point(1, NA, NA)), s2_lnglat(NA, NA))
  expect_identical(as_s2_lnglat(s2_point(NA, 1, NA)), s2_lnglat(NA, NA))
  expect_identical(as_s2_lnglat(s2_point(NA, NA, 1)), s2_lnglat(NA, NA))
})

test_that("s2_point can be imported from s2_geography", {
  expect_equal(
    as_s2_point(as_s2_geography("POINT (-64 45)")),
    as_s2_point(as_s2_lnglat(as_s2_geography("POINT (-64 45)")))
  )
})

test_that("s2_point objects can be printed", {
  expect_output(print(s2_point(1, 2, 3)), "s2_point_crs")
})
