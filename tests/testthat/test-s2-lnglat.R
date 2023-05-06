
test_that("s2_lnglat objects can be created from and converted back to R objects", {
  # in
  expect_s3_class(s2_lnglat(45, 64), "wk_xy")
  expect_length(s2_lnglat(45, 64), 1)
  expect_s3_class(as_s2_lnglat(matrix(c(45, 64), ncol = 2)), "wk_xy")
  lnglat <- s2_lnglat(45, 64)
  expect_identical(as_s2_lnglat(lnglat), lnglat)
  expect_identical(
    as_s2_lnglat(s2_point(1, 0, 0)),
    s2_lnglat(0, 0)
  )

  expect_identical(
    as_s2_lnglat(s2_point(NaN, NaN, NaN)),
    s2_lnglat(NaN, NaN)
  )
})

test_that("s2_lnglat can be imported from s2_geography", {
  expect_equal(
    as_s2_lnglat(as_s2_geography("POINT (-64 45)")),
    s2_lnglat(-64, 45)
  )
})

test_that("s2_lnglat can be imported from wkb", {
  wkb_point <- wk::as_wkb("POINT (-64 45)")

  expect_equal(
    as_s2_lnglat(wkb_point),
    s2_lnglat(-64, 45)
  )
})

test_that("s2_lnglat objects can be printed", {
  expect_output(print(s2_lnglat(-64, 45)), "OGC:CRS84")
})
