
test_that("s2polyline objects can be created from and converted back to R objects", {

  # in
  expect_is(s2polyline(s2latlng(45:50, -64)), "s2polyline")
  expect_is(s2polyline(as.matrix(s2latlng(45:50, -64))), "s2polyline")
  polyline <- s2polyline(s2latlng(45:50, -64))
  expect_identical(s2polyline(polyline), polyline)

  missing_latlng <- new_s2xptr(list(NULL), "s2latlng")
  expect_error(s2polyline(missing_latlng), "Can't create")

  # out
  missing_polyline <- new_s2xptr(list(NULL), "s2polyline")
  expect_equal(
    as.data.frame(s2latlng(s2polyline(s2latlng(45:50, -64)))),
    as.data.frame(s2latlng(45:50, -64))
  )
  expect_equal(
    as.data.frame(s2polyline(s2latlng(45:50, -64))),
    as.data.frame(s2latlng(45:50, -64))
  )
  expect_equal(
    as.matrix(s2polyline(s2latlng(45:50, -64))),
    as.matrix(s2latlng(45:50, -64))
  )
  expect_error(s2latlng(missing_polyline), "Can't convert")
  expect_error(s2latlng(rep(polyline, 2)), "Can't convert")

  expect_output(print(s2polyline(s2latlng(45:50, -64))), "s2polyline")
  expect_output(print(missing_polyline), "NULL")
})
