
test_that("s2_geography is a vctr", {
  skip_if_not_installed("vctrs")

  x <- new_s2_geography(list(NULL))
  expect_true(vctrs::vec_is(x))
  expect_identical(vctrs::vec_data(x), list(NULL))
  expect_identical(vctrs::vec_restore(list(NULL), x), x)
  expect_identical(vctrs::vec_ptype_abbr(x), class(x)[1])
})

test_that("s2_cell is a vctr", {
  skip_if_not_installed("vctrs")

  x <- new_s2_cell(NA_real_)
  expect_true(vctrs::vec_is(x))
  expect_identical(vctrs::vec_data(x), NA_real_)
  expect_identical(vctrs::vec_restore(NA_real_, x), x)
  expect_identical(vctrs::vec_ptype_abbr(x), "s2cell")
})

test_that("s2_cell_union is a vctr", {
  skip_if_not_installed("vctrs")

  x <- new_s2_cell_union(list(NULL))
  expect_true(vctrs::vec_is(x))
  expect_identical(vctrs::vec_data(x), list(NULL))
  expect_identical(vctrs::vec_restore(list(NULL), x), x)
  expect_identical(vctrs::vec_ptype_abbr(x), "s2cellunion")
})
