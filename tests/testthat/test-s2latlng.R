
test_that("s2latlng objects can be created from and converted back to R objects", {
  # in
  expect_is(s2latlng(45, 64), "s2latlng")
  expect_length(s2latlng(45, 64), 1)
  expect_is(s2latlng(matrix(45, 64, ncol = 2)), "s2latlng")
  latlng <- s2latlng(45, 64)
  expect_identical(s2latlng(latlng), latlng)
  expect_identical(
    as.data.frame(s2latlng(s2point(1, 0, 0))),
    as.data.frame(s2latlng(0, 0))
  )

  # out
  expect_identical(as.data.frame(s2latlng(45, 64)), data.frame(lat = 45, lng = 64))
  expect_identical(as.matrix(s2latlng(45, 64)), as.matrix(data.frame(lat = 45, lng = 64)))

  # zero-length in and out
  expect_length(s2latlng(double(), double()), 0)
  expect_identical(
    as.data.frame(s2latlng(double(), double())),
    data.frame(lat = double(), lng = double())
  )

  # NAs and NULLs
  expect_identical(
    as.data.frame(s2latlng(double(), double())[NA]),
    data.frame(lat = NA_real_, lng = NA_real_)
  )
})

test_that("s2latlng can be imported from wkb", {
  wkb_point <- wk::as_wkb("POINT (-64 45)")

  expect_identical(
    as.data.frame(s2latlng(wkb_point)),
    data.frame(lat = 45, lng = -64)
  )
})

test_that("s2latlng objects can be printed", {
  expect_output(print(s2latlng(45, 64)), "s2latlng")
})
