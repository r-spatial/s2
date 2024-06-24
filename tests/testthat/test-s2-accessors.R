
test_that("s2_is_collection works", {
  expect_identical(s2_is_collection(NA_character_), NA)
  expect_false(s2_is_collection("POINT (-64 45)"))
  expect_false(s2_is_collection("POINT EMPTY"))
  expect_true(s2_is_collection("MULTIPOINT ((0 0), (1 1))"))

  expect_false(s2_is_collection("LINESTRING (0 0, 1 1)"))
  expect_true(s2_is_collection("MULTILINESTRING ((0 0, 1 1), (2 2, 3 3))"))
  expect_false(s2_is_collection("POLYGON ((0 0, 10 0, 10 10, 0 10, 0 0))"))
  expect_true(
    s2_is_collection("MULTIPOLYGON (
      ((40 40, 20 45, 45 30, 40 40)),
      ((20 35, 10 30, 10 10, 30 5, 45 20, 20 35), (30 20, 20 15, 20 25, 30 20))
    )")
  )
})

test_that("s2_is_valid() works", {
  expect_identical(
    s2_is_valid(
      c(
        # valid
        "POINT (0 1)", "LINESTRING (0 0, 1 1)", "POLYGON ((0 0, 0 1, 1 0, 0 0))",
        "GEOMETRYCOLLECTION (POINT (0 1))",
        # (for the purposes of S2, linestrings that cross aren't considered invalid
        # in the sense that they won't cause errors when you try to pass them to
        # a boolean operation or the builder)
        # invalid
        "LINESTRING (0 0, 0 0, 1 1)",
        "POLYGON ((0 0, 0 1, 1 0, 0 0, 0 0))",
        "GEOMETRYCOLLECTION (POINT (0 1), POLYGON ((0 0, 0 1, 1 0, 0 0, 0 0)))",
        NA
      )
    ),
    c(TRUE, TRUE, TRUE, TRUE, FALSE, FALSE, FALSE, NA)
  )
})

test_that("s2_is_valid_detail() works", {
  expect_identical(
    s2_is_valid_detail(
      c(
        # valid
        "POINT (0 1)", "LINESTRING (0 0, 1 1)", "POLYGON ((0 0, 0 1, 1 0, 0 0))",
        "GEOMETRYCOLLECTION (POINT (0 1))",
        # (for the purposes of S2, linestrings that cross aren't considered invalid
        # in the sense that they won't cause errors when you try to pass them to
        # a boolean operation or the builder)
        # invalid
        "LINESTRING (0 0, 0 0, 1 1)",
        "POLYGON ((0 0, 0 1, 1 0, 0 0, 0 0))",
        "GEOMETRYCOLLECTION (POINT (0 1), POLYGON ((0 0, 0 1, 1 0, 0 0, 0 0)))",
        NA
      )
    ),
    data.frame(
      is_valid = c(TRUE, TRUE, TRUE, TRUE, FALSE, FALSE, FALSE, NA),
      reason = c(
        NA, NA, NA, NA,
        "Vertices 0 and 1 are identical",
        "Loop 0: Edge 3 is degenerate (duplicate vertex)",
        "Loop 0: Edge 3 is degenerate (duplicate vertex)",
        NA
      ),
      stringsAsFactors = FALSE
    )
  )
})

test_that("s2_dimension works", {
  expect_identical(s2_dimension(NA_character_), NA_integer_)
  expect_identical(s2_dimension("POINT EMPTY"), 0L)
  expect_identical(s2_dimension("LINESTRING EMPTY"), 1L)
  expect_identical(s2_dimension("POLYGON EMPTY"), 2L)
})

test_that("s2_num_points works", {
  expect_identical(s2_num_points(NA_character_), NA_integer_)
  expect_identical(s2_num_points("POINT (-64 45)"), 1L)
  expect_identical(s2_num_points("POINT EMPTY"), 0L)
  expect_identical(s2_num_points("LINESTRING (0 0, 1 1)"), 2L)
  expect_identical(s2_num_points("POLYGON ((0 0, 10 0, 10 10, 0 10, 0 0))"), 4L)
  expect_identical(
    s2_num_points("GEOMETRYCOLLECTION (POINT (0 1), LINESTRING (0 0, 1 1))"),
    3L
  )
})

test_that("s2_is_empty works", {
  expect_identical(s2_is_empty(NA_character_), NA)
  expect_false(s2_is_empty("POINT (-64 45)"))
  expect_true(s2_is_empty("POINT EMPTY"))
  expect_false(s2_is_empty("LINESTRING (0 0, 1 1)"))
  expect_true(s2_is_empty("LINESTRING EMPTY"))
  expect_false(s2_is_empty("POLYGON ((0 0, 10 0, 10 10, 0 10, 0 0))"))
  expect_true(s2_is_empty("POLYGON EMPTY"))
})

test_that("s2_area works", {
  expect_identical(s2_area(NA_character_), NA_real_)
  expect_identical(s2_area("POINT (-64 45)"), 0)
  expect_identical(s2_area("POINT EMPTY"), 0)
  expect_identical(s2_area("LINESTRING (0 0, 1 1)"), 0)
  expect_identical(s2_area("POLYGON EMPTY"), 0)
  expect_equal(s2_area("POLYGON ((0 0, 90 0, 0 90, 0 0))", radius = 1), 4 * pi / 8)
  # make sure the radius is squared!
  expect_true(
    abs(s2_area("POLYGON ((0 0, 10 0, 10 10, 0 10, 0 0))", radius = 180 / pi) - 100) < 0.27
  )

  expect_identical(
    s2_area("POLYGON ((0 0, 90 0, 0 90, 0 0))"),
    s2_area("GEOMETRYCOLLECTION(POLYGON ((0 0, 90 0, 0 90, 0 0)))")
  )
})

test_that("s2_length works", {
  expect_identical(s2_length(NA_character_), NA_real_)
  expect_identical(s2_length("POINT (-64 45)"), 0)
  expect_identical(s2_length("POINT EMPTY"), 0)
  expect_identical(s2_length("LINESTRING EMPTY"), 0)
  expect_identical(s2_length("LINESTRING (0 0, 0 1)", radius = 180 / pi), 1)
  expect_identical(s2_length("POLYGON ((0 0, 10 0, 10 10, 0 10, 0 0))"), 0)
})

test_that("s2_perimeter works", {
  expect_identical(s2_perimeter(NA_character_), NA_real_)
  expect_identical(s2_perimeter("POINT (-64 45)"), 0)
  expect_identical(s2_perimeter("POINT EMPTY"), 0)
  expect_identical(s2_perimeter("LINESTRING EMPTY"), 0)
  expect_identical(s2_perimeter("LINESTRING (0 0, 1 1)"), 0)

  # there is some error here because of the way this is calculated involves
  # some round-tripping through lat/lon
  expect_true(
    abs(s2_perimeter("POLYGON ((0 0, 10 0, 10 10, 0 10, 0 0))", radius = 180 / pi) - 40) < 0.155
  )
})

test_that("s2_x and s2_y works", {
  expect_identical(s2_x(NA_character_), NA_real_)
  expect_identical(s2_y(NA_character_), NA_real_)
  expect_equal(s2_x("POINT (-64 45)"), -64)
  expect_equal(s2_y("POINT (-64 45)"), 45)
  expect_identical(s2_x("POINT EMPTY"), NaN)
  expect_identical(s2_y("POINT EMPTY"), NaN)
  expect_error(s2_x("LINESTRING EMPTY"), "Can't compute")
  expect_error(s2_y("LINESTRING EMPTY"), "Can't compute")
  expect_error(s2_x("POLYGON EMPTY"), "Can't compute")
  expect_error(s2_y("POLYGON EMPTY"), "Can't compute")
})

test_that("s2_project() and s2_project_normalized() work", {
  expect_equal(
    s2_project(
      "LINESTRING (0 0, 0 90)",
      c("POINT (0 0)", "POINT (0 22.5)", "POINT (0 67.5)", "POINT (0 90)", NA),
      radius = 1
    ),
    c(0, 0.25, 0.75, 1, NA_real_) * pi / 2
  )

  expect_equal(
    s2_project_normalized(
      "LINESTRING (0 0, 0 90)",
      c("POINT (0 0)", "POINT (0 22.5)", "POINT (0 67.5)", "POINT (0 90)", "POINT EMPTY", NA)
    ),
    c(0, 0.25, 0.75, 1, NaN, NA_real_)
  )

  expect_identical(
    s2_project_normalized("POINT (0 1)", "POINT (0 1)"),
    NaN
  )
  expect_identical(
    s2_project_normalized("LINESTRING (0 1, 1 1)", "LINESTRING (0 1, 1 1)"),
    NaN
  )
  expect_identical(
    s2_project_normalized("LINESTRING (0 1, 1 1)", "MULTIPOINT (0 1, 1 1)"),
    NaN
  )
})

test_that("s2_distance works", {
  expect_equal(
    s2_distance("POINT (0 0)", "POINT (90 0)", radius = 180 / pi),
    90
  )

  expect_equal(
    s2_distance("POINT (0 0)", "LINESTRING (90 0, 91 0)", radius = 180 / pi),
    90
  )

  expect_equal(
    s2_distance("POINT (0 0)", "POLYGON ((90 0, 91 0, 92 1, 90 0))", radius = 180 / pi),
    90
  )

  expect_identical(s2_distance("POINT (0 0)", NA_character_), NA_real_)
  expect_identical(s2_distance(NA_character_, "POINT (0 0)"), NA_real_)
  expect_identical(s2_distance("POINT (0 0)", "POINT EMPTY"), NA_real_)
  expect_identical(s2_distance("POINT EMPTY", "POINT (0 0)"), NA_real_)
})

test_that("s2_max_distance works", {
  expect_equal(
    s2_max_distance("POINT (0 0)", "POINT (90 0)", radius = 180 / pi),
    90
  )

  expect_equal(
    s2_max_distance("POINT (0 0)", "LINESTRING (90 0, 91 0)", radius = 180 / pi),
    91
  )

  expect_equal(
    s2_max_distance("POINT (0 0)", "POLYGON ((90 0, 91 0, 89 1, 90 0))", radius = 180 / pi),
    91
  )

  expect_identical(s2_max_distance("POINT (0 0)", NA_character_), NA_real_)
  expect_identical(s2_max_distance(NA_character_, "POINT (0 0)"), NA_real_)
  expect_identical(s2_max_distance("POINT (0 0)", "POINT EMPTY"), NA_real_)
  expect_identical(s2_max_distance("POINT EMPTY", "POINT (0 0)"), NA_real_)
})
