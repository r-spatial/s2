
test_that("s2_geography is a vctr", {
  x <- new_s2_xptr(list(NULL), "s2_geography")
  expect_true(vctrs::vec_is(x))
  expect_identical(vctrs::vec_data(x), list(NULL))
  expect_identical(vctrs::vec_restore(list(NULL), x), x)
  expect_identical(vctrs::vec_ptype_abbr(x), class(x)[1])
})

test_that("s2_latlng is a vctr", {
  x <- new_s2_xptr(list(NULL), "s2_latlng")
  expect_true(vctrs::vec_is(x))
  expect_identical(vctrs::vec_data(x), list(NULL))
  expect_identical(vctrs::vec_restore(list(NULL), x), x)
  expect_identical(vctrs::vec_ptype_abbr(x), class(x)[1])
})

test_that("s2_point is a vctr", {
  x <- new_s2_xptr(list(NULL), "s2_point")
  expect_true(vctrs::vec_is(x))
  expect_identical(vctrs::vec_data(x), list(NULL))
  expect_identical(vctrs::vec_restore(list(NULL), x), x)
  expect_identical(vctrs::vec_ptype_abbr(x), class(x)[1])
})
