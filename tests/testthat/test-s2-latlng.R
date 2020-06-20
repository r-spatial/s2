
test_that("s2_latlng objects can be created from and converted back to R objects", {
  # in
  expect_is(s2_latlng(45, 64), "s2_latlng")
  expect_length(s2_latlng(45, 64), 1)
  expect_is(as_s2_latlng(matrix(45, 64, ncol = 2)), "s2_latlng")
  latlng <- s2_latlng(45, 64)
  expect_identical(as_s2_latlng(latlng), latlng)
  expect_identical(
    as.data.frame(as_s2_latlng(s2_point(1, 0, 0))),
    as.data.frame(s2_latlng(0, 0))
  )

  # subset assignment
  latlng2 <- latlng
  latlng2[1] <- latlng
  expect_identical(latlng2, latlng)

  latlng2 <- latlng
  latlng2[[1]] <- latlng
  expect_identical(latlng2, latlng)

  # out
  expect_identical(as.data.frame(s2_latlng(45, 64)), data.frame(lat = 45, lng = 64))
  expect_identical(as.matrix(s2_latlng(45, 64)), as.matrix(data.frame(lat = 45, lng = 64)))

  # zero-length in and out
  expect_length(s2_latlng(double(), double()), 0)
  expect_identical(
    as.data.frame(s2_latlng(double(), double())),
    data.frame(lat = double(), lng = double())
  )

  # NAs and NULLs
  expect_identical(
    as.data.frame(s2_latlng(double(), double())[NA]),
    data.frame(lat = NA_real_, lng = NA_real_)
  )
})

test_that("s2_latlng can be imported/exported to/from wkb and wkt", {
  wkb_point <- wk::as_wkb("POINT (-64 45)")
  expect_identical(wk::as_wkb(s2_latlng(45, -64), endian = 1), wkb_point)
  expect_identical(wk::as_wkt(s2_latlng(45, -64)), wk::wkt("POINT (-64 45)"))
  expect_identical(wk::as_wkt(s2_latlng(NA, NA)), wk::wkt("POINT EMPTY"))

  expect_equal(
    as.data.frame(as_s2_latlng(wk::as_wkb(c("POINT EMPTY", "POINT (-64 45)")))),
    data.frame(lat = c(NA, 45), lng = c(NA, -64))
  )

  expect_identical(
    as.data.frame(as_s2_latlng(c("POINT EMPTY", "POINT (-64 45)"))),
    data.frame(lat = c(NA, 45), lng = c(NA, -64))
  )

  expect_identical(
    as.data.frame(as_s2_latlng(wk::wkt(c("POINT EMPTY", "POINT (-64 45)")))),
    data.frame(lat = c(NA, 45), lng = c(NA, -64))
  )

  expect_error(as_s2_latlng(wk::wkt("LINESTRING EMPTY")), "Can't import non-points")
  expect_error(as_s2_latlng(wk::as_wkb("LINESTRING EMPTY")), "Can't import non-points")
})

test_that("s2_latlng vectors can't have other types of objects concatenated or asssigned", {
  latlng <- new_s2_xptr(list(NULL), class = "s2_latlng")
  expect_is(c(latlng, latlng), "s2_latlng")
  expect_error(c(latlng, new_s2_xptr(list(), class = "some_other_class")), "All items must inherit")
  expect_error(latlng[1] <- new_s2_xptr(list(NULL), class = "some_other_class"), "no applicable method")
  expect_error(latlng[[1]] <- new_s2_xptr(list(NULL), class = "some_other_class"), "no applicable method")
})

test_that("s2_latlng can be imported from s2_geography", {
  expect_equal(
    as.data.frame(as_s2_latlng(as_s2_geography("POINT (-64 45)"))),
    data.frame(lat = 45, lng = -64)
  )
})

test_that("s2_latlng can be imported from wkb", {
  wkb_point <- wk::as_wkb("POINT (-64 45)")

  expect_identical(
    as.data.frame(as_s2_latlng(wkb_point)),
    data.frame(lat = 45, lng = -64)
  )
})

test_that("s2_latlng objects can be printed", {
  expect_output(print(s2_latlng(45, 64)), "s2_latlng")
})
