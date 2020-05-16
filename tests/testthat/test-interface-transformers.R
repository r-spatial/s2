
test_that("s2_intersection() works", {
  expect_equal(s2_x(s2_intersection("POINT (30 10)", "POINT (30 10)")), 30)
  expect_equal(s2_y(s2_intersection("POINT (30 10)", "POINT (30 10)")), 10)
  expect_true(s2_isempty(s2_intersection("POINT (30 10)", "POINT (30 11)")))
})

test_that("s2_union(x) works", {
  expect_equal(s2_x(s2_union("POINT (30 10)")), 30)
  expect_equal(s2_y(s2_union("POINT (30 10)")), 10)
  expect_true(s2_isempty(s2_union("POINT EMPTY")))
})

test_that("s2_union(x, y) works", {
  expect_equal(s2_x(s2_union("POINT (30 10)", "POINT EMPTY")), 30)
  expect_equal(s2_y(s2_union("POINT (30 10)", "POINT EMPTY")), 10)
  expect_true(s2_isempty(s2_union("POINT EMPTY", "POINT EMPTY")))
})
