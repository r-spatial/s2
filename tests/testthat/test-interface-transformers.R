
test_that("s2_closestpoint() works", {
  expect_equal(s2_x(s2_closestpoint("POINT (0 1)", "POINT (30 10)")), 0)
  expect_equal(s2_y(s2_closestpoint("POINT (0 1)", "POINT (30 10)")), 1)
  expect_true(s2_isempty(s2_closestpoint("POINT (30 10)", "POINT EMPTY")))
})

test_that("s2_difference() works", {
  expect_equal(s2_x(s2_difference("POINT (30 10)", "POINT EMPTY")), 30)
  expect_equal(s2_y(s2_difference("POINT (30 10)", "POINT EMPTY")), 10)
  expect_true(s2_isempty(s2_difference("POINT (30 10)", "POINT (30 10)")))
})

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

test_that("s2_union_agg() works", {
  expect_equal(s2_x(s2_union_agg(c("POINT (30 10)", "POINT EMPTY"))), 30)
  expect_equal(s2_y(s2_union_agg(c("POINT (30 10)", "POINT EMPTY"))), 10)
  expect_true(s2_isempty(s2_union_agg(c("POINT EMPTY", "POINT EMPTY"))))

  # NULL handling
  expect_identical(
    s2_union_agg(c("POINT (30 10)", NA), na.rm = FALSE),
    s2geography(NA_character_)
  )
  expect_equal(
    s2_x(s2_union_agg(c("POINT (30 10)", NA), na.rm = TRUE)),
    30
  )
})
