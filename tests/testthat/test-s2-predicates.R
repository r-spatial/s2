
test_that("s2_contains() works", {
  expect_identical(s2_contains("POINT (0 0)", NA_character_), NA)

  expect_true(s2_contains("POINT (0 0)", "POINT (0 0)"))
  expect_false(s2_contains("POINT (0 0)", "POINT (1 1)"))
  expect_true(s2_contains("POINT (0 0)", "POINT EMPTY")) # surprising and not true in sf

  # make sure model is passed on to at least one binary predicate
  # in the open model, lines do not contain endpoints (but do contain other points)
  expect_false(s2_contains("LINESTRING (0 0, 0.5 0.5, 1 1)", "POINT (0 0)", model = 0))
  expect_true(s2_contains("LINESTRING (0 0, 0.5 0.5, 1 1)", "POINT (0.5 0.5)", model = 0))
  expect_false(s2_contains("LINESTRING (0 0, 0.5 0.5, 1 1)", "POINT (0.25 0.25)", model = 0))

  # semi-open and closed: endpoints are contained
  expect_true(s2_contains("LINESTRING (0 0, 0.5 0.5, 1 1)", "POINT (0 0)", model = 1))
  expect_true(s2_contains("LINESTRING (0 0, 0.5 0.5, 1 1)", "POINT (0.5 0.5)", model = 1))
  expect_false(s2_contains("LINESTRING (0 0, 0.5 0.5, 1 1)", "POINT (0.25 0.25)", model = 1))
  expect_true(s2_contains("LINESTRING (0 0, 0.5 0.5, 1 1)", "POINT (0 0)", model = 2))
  expect_true(s2_contains("LINESTRING (0 0, 0.5 0.5, 1 1)", "POINT (0.5 0.5)", model = 2))
  expect_false(s2_contains("LINESTRING (0 0, 0.5 0.5, 1 1)", "POINT (0.25 0.25)", model = 2))
})

test_that("s2_covers() and s2_covered_by() work", {
  expect_true(s2_covers("POINT (0 0)", "POINT (0 0)"))
  expect_false(s2_covers("POINT (0 0)", "POINT (1 1)"))
  warning("Surprising result: POINT (0 0) covers POINT EMPTY")
  expect_true(s2_covers("POINT (0 0)", "POINT EMPTY"))
  expect_true(s2_covers("LINESTRING (0 0, 1 1)", "POINT (0 0)"))
  expect_false(s2_covers("LINESTRING (0 0, 1 1)", "POINT (-1 -1)"))

  polygon <- "POLYGON ((0 0, 1 1, 0 1, 0 0))"
  line <- "LINESTRING (0.1 0.1, 0.9 0.9)"
  point <- "POINT (0.5 0.7)"

  expect_true(s2_covers(polygon, polygon))
  warning(sprintf("Surprising result: %s covers %s with all three models", polygon, line))
  expect_false(s2_covers(polygon, line, model = 0))
  expect_false(s2_covers(polygon, line, model = 1))
  expect_false(s2_covers(polygon, line, model = 2))
  expect_true(s2_covers(polygon, point, model = 0))
  expect_true(s2_covers(polygon, point, model = 1))
  expect_true(s2_covers(polygon, point, model = 2))
  expect_false(s2_covered_by(polygon, line, model = 0))
  expect_false(s2_covered_by(polygon, line, model = 1))
  expect_false(s2_covered_by(polygon, line, model = 2))
  expect_true(s2_covered_by(point, polygon, model = 0))
  expect_true(s2_covered_by(point, polygon, model = 1))
  expect_true(s2_covered_by(point, polygon, model = 2))
})

test_that("s2_disjoint() works", {
  expect_identical(s2_disjoint("POINT (0 0)", NA_character_), NA)

  expect_false(s2_disjoint("POINT (0 0)", "POINT (0 0)"))
  expect_true(s2_disjoint("POINT (0 0)", "POINT (1 1)"))
  expect_true(s2_disjoint("POINT (0 0)", "POINT EMPTY"))

  expect_false(s2_disjoint("LINESTRING (0 0, 1 1)", "POINT (0 0)"))
  expect_true(s2_disjoint("LINESTRING (0 0, 1 1)", "POINT (-1 -1)"))
})

test_that("s2_equals() works", {
  expect_identical(s2_equals("POINT (0 0)", NA_character_), NA)

  expect_true(s2_equals("POINT (0 0)", "POINT (0 0)"))
  expect_true(s2_equals("POINT (0 0)", "POINT (0 0)", model = 0))
  expect_true(s2_equals("POINT (0 0)", "POINT (0 0)", model = 1))
  expect_true(s2_equals("POINT (0 0)", "POINT (0 0)", model = 2))
  expect_false(s2_equals("POINT (0 0)", "POINT (1 1)"))
  expect_false(s2_equals("POINT (0 0)", "POINT EMPTY"))
  expect_true(s2_equals("POINT EMPTY", "POINT EMPTY"))

  expect_true(s2_equals("LINESTRING (0 0, 1 1)", "LINESTRING (1 1, 0 0)"))
  expect_false(s2_equals("LINESTRING (0 1, 1 1)", "LINESTRING (1 1, 0 0)"))
})

test_that("s2_intersects() works", {
  expect_identical(s2_intersects("POINT (0 0)", NA_character_), NA)

  expect_true(s2_intersects("POINT (0 0)", "POINT (0 0)"))
  expect_false(s2_intersects("POINT (0 0)", "POINT (1 1)"))
  expect_false(s2_intersects("POINT (0 0)", "POINT EMPTY"))

  expect_true(s2_intersects("LINESTRING (0 0, 1 1)", "LINESTRING (1 0, 0 1)"))
  expect_false(s2_intersects("LINESTRING (0 0, 1 1)", "LINESTRING (-2 -2, -1 -1)"))
  expect_true(s2_intersects("LINESTRING (0 0, 1 1)", "POINT (0 0)"))
  expect_false(s2_intersects("LINESTRING (0 0, 1 1)", "POINT (0 0)", model = 0))
  expect_true(s2_intersects("LINESTRING (0 0, 1 1)", "POINT (0 0)", model = 1))
  expect_true(s2_intersects("LINESTRING (0 0, 1 1)", "POINT (0 0)", model = 2))
  polygon = "POLYGON((0 0,1 0,1 1,0 1,0 0))"
  expect_false(s2_intersects(polygon, "POINT (0 0)"))
  expect_false(s2_intersects(polygon, "POINT (0 0)", model = 0))
  expect_false(s2_intersects(polygon, "POINT (0 0)", model = 1))
  expect_true(s2_intersects(polygon, "POINT (0 0)", model = 2))
})

test_that("s2_intersects_box() works", {
  expect_error(
    s2_intersects_box("POINT (-1 -1)", -2, -2, 2, 2, detail = 0),
    "Can't create polygon"
  )
  expect_false(s2_intersects_box("POINT (0 0)", -1, 1, 0, 0))
  expect_false(s2_intersects_box("POINT (0 0)", -1, 0, 1, 0))
  expect_false(s2_intersects_box("POINT (0 0)", 0, -1, 0, 1))
  expect_false(s2_intersects_box("POINT (0 0)", -1, 1, 0, 0, model = 0))
  expect_false(s2_intersects_box("POINT (0 0)", -1, 1, 0, 0, model = 1))
  expect_false(s2_intersects_box("POINT (0 0)", -1, 1, 0, 0, model = 2)) # FIXME: why is this FALSE?

  expect_true(s2_intersects_box("POINT (-1 -1)", -2, -2, 2, 2))
  expect_false(s2_intersects_box("POINT (-1 -1)", 0, 0, 2, 2))
  expect_false(s2_intersects_box("POINT (0 0)", 1, 1, 2, 2))
})

test_that("s2_within() works", {
  expect_identical(s2_within("POINT (0 0)", NA_character_), NA)

  expect_true(s2_within("POINT (0 0)", "POINT (0 0)"))
  expect_false(s2_within("POINT (0 0)", "POINT (1 1)"))
  expect_false(s2_within("POINT (0 0)", "POINT EMPTY"))

  expect_true(s2_within("POINT (0 0)", "LINESTRING (0 0, 1 1)", model = 1))
  expect_false(s2_within("POINT (0 0)", "LINESTRING (0 0, 1 1)", model = 0))
  expect_false(s2_within("POINT (0 0)", "LINESTRING (1 1, 2 2)"))
})

test_that("s2_dwithin() works", {
  expect_identical(s2_dwithin("POINT (0 0)", NA_character_, 0), NA)

  expect_true(s2_dwithin("POINT (0 0)", "POINT (90 0)", pi / 2 + 0.01, radius = 1))
  expect_false(s2_dwithin("POINT (0 0)", "POINT (90 0)", pi / 2 - 0.01, radius = 1))
  expect_false(s2_dwithin("POINT (0 0)", "POINT EMPTY", 0))

  expect_true(s2_dwithin("LINESTRING (-45 0, 45 0)", "POINT (0 20)", 21, radius = 180 / pi))
  expect_false(s2_dwithin("LINESTRING (-45 0, 45 0)", "POINT (0 20)", 19, radius = 180 / pi))

  # check vectorization
  expect_identical(
    s2_dwithin("POINT (0 0)", "POINT (90  0)", pi / 2 + c(0.01, -0.01), radius = 1),
    c(TRUE, FALSE)
  )
})
