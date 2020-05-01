
test_that("s2point objects can be created from and converted back to R objects", {
  # in
  expect_is(s2point(1, 2, 3), "s2point")
  expect_length(s2point(1, 2, 3), 1)
  expect_is(s2point(matrix(1, 2, 3, ncol = 3)), "s2point")
  point <- s2point(1, 2, 3)
  expect_identical(s2point(point), point)

  # out
  expect_identical(as.data.frame(s2point(1, 2, 3)), data.frame(x = 1, y = 2, z = 3))
  expect_identical(as.matrix(s2point(1, 2, 3)), as.matrix(data.frame(x = 1, y = 2, z = 3)))

  # zero-length
  expect_length(s2point(double(), double(), double()), 0)
  expect_identical(
    as.data.frame(s2point(double(), double(), double())),
    data.frame(x = double(), y = double(), z = double())
  )
})

test_that("s2point objects can be printed", {
  expect_output(print(s2point(1, 2, 3)), "s2point")
})
