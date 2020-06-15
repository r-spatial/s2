
test_that("s2_latlng objects can be created from and converted back to R objects", {
  # in
  expect_is(s2_latlng(45, 64), "s2_latlng")
  expect_length(s2_latlng(45, 64), 1)
  expect_is(s2_latlng(matrix(45, 64, ncol = 2)), "s2_latlng")
  latlng <- s2_latlng(45, 64)
  expect_identical(s2_latlng(latlng), latlng)
  expect_identical(
    as.data.frame(s2_latlng(s2_point(1, 0, 0))),
    as.data.frame(s2_latlng(0, 0))
  )

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

test_that("s2_latlng can be imported from wkb", {
  wkb_point <- wk::as_wkb("POINT (-64 45)")

  expect_identical(
    as.data.frame(s2_latlng(wkb_point)),
    data.frame(lat = 45, lng = -64)
  )
})

test_that("s2_latlng objects can be printed", {
  expect_output(print(s2_latlng(45, 64)), "s2_latlng")
})
