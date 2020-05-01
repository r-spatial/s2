
test_that("s2latlng objects can be created from and converted back to R objects", {
  expect_is(s2latlng(45, 64), "s2latlng")
  expect_length(s2latlng(45, 64), 1)
  expect_is(s2latlng(matrix(45, 64, ncol = 2)), "s2latlng")
  expect_identical(as.data.frame(s2latlng(45, 64)), data.frame(lat = 45, lng = 64))
  expect_identical(
    data.frame(lat = 45, lng = 64),
    as.data.frame(s2latlng(s2latlng(45, 64)))
  )

  # zero-length
  expect_length(s2latlng(double(), double()), 0)
})

test_that("s2latlng objects can be printed", {
  expect_output(print(s2latlng(45, 64)), "s2latlng")
})
