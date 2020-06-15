
test_that("s2_xptr class works", {
  expect_s3_class(new_s2_xptr(list()), "s2_xptr")
  expect_s3_class(new_s2_xptr(list(), class = "custom"), "custom")
  expect_s3_class(new_s2_xptr(list(), class = "custom"), "s2_xptr")
  expect_error(new_s2_xptr(NULL), "must be a bare list")
})

test_that("objects pointed to by an s2_xptr are destroyed by the garbage collector", {
  xptr <- expect_output(s2_xptr_test(1), "Allocating")
  expect_output(s2_xptr_test_op(xptr), "test\\(\\) on XPtrTest")
  expect_identical(validate_s2_xptr(xptr), xptr)
  expect_output({rm(xptr); gc()}, "Destroying")
})

test_that("s2_xptr validation works", {
  expect_identical(validate_s2_xptr(new_s2_xptr(list())), new_s2_xptr(list()))
  expect_identical(validate_s2_xptr(new_s2_xptr(list(NULL))), new_s2_xptr(list(NULL)))
  expect_error(validate_s2_xptr(list("wrong type")), "must be externalptr")
})

test_that("s2_xptr subsetting and concatenation work", {
  xptr <- new_s2_xptr(list(NULL, NULL))
  expect_identical(xptr[1], new_s2_xptr(list(NULL)))
  expect_identical(xptr[[1]], xptr[1])
  expect_identical(c(xptr, xptr), new_s2_xptr(list(NULL, NULL, NULL, NULL)))
  expect_identical(rep(xptr, 2), new_s2_xptr(list(NULL, NULL, NULL, NULL)))
  expect_identical(rep_len(xptr, 4), new_s2_xptr(list(NULL, NULL, NULL, NULL)))
})

test_that("s2_xptr default print method works", {
  expect_output(print(new_s2_xptr()), "s2_xptr")
  expect_output(print(new_s2_xptr(list(NULL))), "s2_xptr")
})
