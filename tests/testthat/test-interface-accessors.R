
test_that("s2_iscollection works", {
  expect_false(s2_iscollection("POINT (-64 45)"))
  expect_false(s2_iscollection("POINT EMPTY"))
})

test_that("s2_dimension works", {
  expect_identical(s2_dimension("POINT (-64 45)"), 0L)
  expect_identical(s2_dimension("POINT EMPTY"), 0L)
})

test_that("s2_numpoints works", {
  expect_identical(s2_numpoints("POINT (-64 45)"), 1L)
  expect_identical(s2_numpoints("POINT EMPTY"), 0L)
})

test_that("s2_area works", {
  expect_identical(s2_area("POINT (-64 45)"), 0)
  expect_identical(s2_area("POINT EMPTY"), 0)
})

test_that("s2_length works", {
  expect_identical(s2_length("POINT (-64 45)"), 0)
  expect_identical(s2_length("POINT EMPTY"), 0)
})

test_that("s2_perimeter works", {
  expect_identical(s2_perimeter("POINT (-64 45)"), 0)
  expect_identical(s2_perimeter("POINT EMPTY"), 0)
})

test_that("s2_x and s2_y works", {
  expect_identical(s2_x("POINT (-64 45)"), -64)
  expect_identical(s2_y("POINT (-64 45)"), 45)
  expect_identical(s2_x("POINT EMPTY"), NA_real_)
  expect_identical(s2_y("POINT EMPTY"), NA_real_)
})

