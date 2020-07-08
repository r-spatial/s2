
test_that("s2_point objects can be created from and converted back to R objects", {
  # in
  expect_is(s2_point(1, 2, 3), "s2_point")
  expect_length(s2_point(1, 2, 3), 1)
  expect_is(as_s2_point(matrix(1, 2, 3, ncol = 3)), "s2_point")
  point <- s2_point(1, 2, 3)
  expect_identical(as_s2_point(point), point)
  expect_identical(
    as.data.frame(as_s2_point(s2_lnglat(0, 0))),
    as.data.frame(s2_point(1, 0, 0))
  )

  # subset assignment
  point2 <- point
  point2[1] <- point
  expect_identical(point2, point)

  point2 <- point
  point2[[1]] <- point
  expect_identical(point2, point)

  # out
  expect_identical(as.data.frame(s2_point(1, 2, 3)), data.frame(x = 1, y = 2, z = 3))
  expect_identical(as.matrix(s2_point(1, 2, 3)), as.matrix(data.frame(x = 1, y = 2, z = 3)))

  # zero-length
  expect_length(s2_point(double(), double(), double()), 0)
  expect_identical(
    as.data.frame(s2_point(double(), double(), double())),
    data.frame(x = double(), y = double(), z = double())
  )

  expect_identical(
    as.data.frame(s2_point(double(), double(), double())[NA]),
    data.frame(x = NA_real_, y = NA_real_, z = NA_real_)
  )
})

test_that("s2_point vectors can't have other types of objects concatenated or asssigned", {
  point <- new_s2_xptr(list(NULL), class = "s2_point")
  expect_is(c(point, point), "s2_point")
  expect_error(c(point, new_s2_xptr(list(), class = "some_other_class")), "All items must inherit")
  expect_error(point[1] <- new_s2_xptr(list(NULL), class = "some_other_class"), "no applicable method")
  expect_error(point[[1]] <- new_s2_xptr(list(NULL), class = "some_other_class"), "no applicable method")
})

test_that("s2_point can be imported from s2_geography", {
  expect_equal(
    as.data.frame(as_s2_point(as_s2_geography("POINT (-64 45)"))),
    as.data.frame(as_s2_point(as_s2_lnglat(as_s2_geography("POINT (-64 45)")))),
  )
})

test_that("s2_point objects can be printed", {
  expect_output(print(s2_point(1, 2, 3)), "s2_point")
})
