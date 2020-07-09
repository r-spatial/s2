
test_that("s2_lnglat objects can be created from and converted back to R objects", {
  # in
  expect_is(s2_lnglat(45, 64), "s2_lnglat")
  expect_length(s2_lnglat(45, 64), 1)
  expect_is(as_s2_lnglat(matrix(45, 64, ncol = 2)), "s2_lnglat")
  lnglat <- s2_lnglat(45, 64)
  expect_identical(as_s2_lnglat(lnglat), lnglat)
  expect_identical(
    as.data.frame(as_s2_lnglat(s2_point(1, 0, 0))),
    as.data.frame(s2_lnglat(0, 0))
  )

  # subset assignment
  lnglat2 <- lnglat
  lnglat2[1] <- lnglat
  expect_identical(lnglat2, lnglat)

  lnglat2 <- lnglat
  lnglat2[[1]] <- lnglat
  expect_identical(lnglat2, lnglat)

  # out
  expect_identical(as.data.frame(s2_lnglat(-64, 45)), data.frame(lng = -64, lat = 45))
  expect_identical(as.matrix(s2_lnglat(-64, 45)), as.matrix(data.frame(lng = -64, lat = 45)))

  # zero-length in and out
  expect_length(s2_lnglat(double(), double()), 0)
  expect_identical(
    as.data.frame(s2_lnglat(double(), double())),
    data.frame(lng = double(), lat = double())
  )

  # NAs and NULLs
  expect_identical(
    as.data.frame(s2_lnglat(double(), double())[NA]),
    data.frame(lng = NA_real_, lat = NA_real_)
  )
})

test_that("s2_lnglat can be imported/exported to/from wkb and wkt", {
  wkb_point <- wk::as_wkb("POINT (-64 45)")
  expect_wkt_equal(wk::as_wkb(s2_lnglat(-64, 45), endian = 1), wkb_point)
  expect_wkt_equal(wk::as_wkt(s2_lnglat(-64, 45)), wk::wkt("POINT (-64 45)"))
  expect_identical(wk::as_wkt(s2_lnglat(NA, NA)), wk::wkt("POINT EMPTY"))

  expect_equal(
    as.data.frame(as_s2_lnglat(wk::as_wkb(c("POINT EMPTY", "POINT (-64 45)")))),
    data.frame(lng = c(NA, -64), lat = c(NA, 45))
  )

  expect_equal(
    as.data.frame(as_s2_lnglat(c("POINT EMPTY", "POINT (-64 45)"))),
    data.frame(lng = c(NA, -64), lat = c(NA, 45))
  )

  expect_equal(
    as.data.frame(as_s2_lnglat(wk::wkt(c("POINT EMPTY", "POINT (-64 45)")))),
    data.frame(lng = c(NA, -64), lat = c(NA, 45))
  )

  expect_error(as_s2_lnglat(wk::wkt("LINESTRING EMPTY")), "non-point")
  expect_error(as_s2_lnglat(wk::as_wkb("LINESTRING EMPTY")), "non-point")
})

test_that("s2_lnglat vectors can't have other types of objects concatenated or asssigned", {
  lnglat <- new_s2_xptr(list(NULL), class = "s2_lnglat")
  expect_is(c(lnglat, lnglat), "s2_lnglat")
  expect_error(c(lnglat, new_s2_xptr(list(), class = "some_other_class")), "All items must inherit")
  expect_error(lnglat[1] <- new_s2_xptr(list(NULL), class = "some_other_class"), "no applicable method")
  expect_error(lnglat[[1]] <- new_s2_xptr(list(NULL), class = "some_other_class"), "no applicable method")
})

test_that("s2_lnglat can be imported from s2_geography", {
  expect_equal(
    as.data.frame(as_s2_lnglat(as_s2_geography("POINT (-64 45)"))),
    data.frame(lng = -64, lat = 45)
  )
})

test_that("s2_lnglat can be imported from wkb", {
  wkb_point <- wk::as_wkb("POINT (-64 45)")

  expect_equal(
    as.data.frame(as_s2_lnglat(wkb_point)),
    data.frame(lng = -64, lat = 45)
  )
})

test_that("s2_lnglat objects can be printed", {
  expect_output(print(s2_lnglat(-64, 45)), "s2_lnglat")
})
