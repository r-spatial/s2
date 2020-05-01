
test_that("s2polygon objects can be created from and converted back to R objects", {
  latlng_loop <- s2latlng(c(0, 10, 0), c(0, 0, 10))
  polyline_loop <- s2polyline(latlng_loop)

  # in
  expect_is(s2polygon(polyline_loop), "s2polygon")
  expect_is(s2polygon(polyline_loop, oriented = TRUE), "s2polygon")
  expect_error(s2polygon(s2polyline(latlng_loop[1:2]), check = TRUE), "must have at least 3 vertices")
  expect_silent(s2polygon(s2polyline(latlng_loop[1:2]), check = FALSE))
  polygon <- s2polygon(polyline_loop)
  expect_identical(s2polygon(polygon), polygon)

  # out
  missing_polygon <- new_s2xptr(list(NULL), "s2polygon")


  expect_output(print(s2polygon(polyline_loop)), "s2polygon")
  expect_output(print(missing_polygon), "NULL")
})
