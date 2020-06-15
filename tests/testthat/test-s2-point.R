
test_that("s2_point objects can be created from and converted back to R objects", {
  # in
  expect_is(as_s2_point(1, 2, 3), "s2_point")
  expect_length(as_s2_point(1, 2, 3), 1)
  expect_is(as_s2_point(matrix(1, 2, 3, ncol = 3)), "s2_point")
  point <- as_s2_point(1, 2, 3)
  expect_identical(as_s2_point(point), point)
  expect_identical(
    as.data.frame(as_s2_point(as_s2_latlng(0, 0))),
    as.data.frame(as_s2_point(1, 0, 0))
  )

  # out
  expect_identical(as.data.frame(as_s2_point(1, 2, 3)), data.frame(x = 1, y = 2, z = 3))
  expect_identical(as.matrix(as_s2_point(1, 2, 3)), as.matrix(data.frame(x = 1, y = 2, z = 3)))

  # zero-length
  expect_length(as_s2_point(double(), double(), double()), 0)
  expect_identical(
    as.data.frame(as_s2_point(double(), double(), double())),
    data.frame(x = double(), y = double(), z = double())
  )

  expect_identical(
    as.data.frame(as_s2_point(double(), double(), double())[NA]),
    data.frame(x = NA_real_, y = NA_real_, z = NA_real_)
  )
})

test_that("s2_point objects can be printed", {
  expect_output(print(as_s2_point(1, 2, 3)), "s2_point")
})
