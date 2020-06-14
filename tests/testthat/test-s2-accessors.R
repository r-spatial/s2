
test_that("s2_iscollection works", {
  expect_identical(s2_iscollection(NA_character_), NA)
  expect_false(s2_iscollection("POINT (-64 45)"))
  expect_false(s2_iscollection("POINT EMPTY"))
  expect_true(s2_iscollection("MULTIPOINT ((0 0), (1 1))"))

  expect_false(s2_iscollection("LINESTRING (0 0, 1 1)"))
  expect_true(s2_iscollection("MULTILINESTRING ((0 0, 1 1), (2 2, 3 3))"))
  expect_false(s2_iscollection("POLYGON ((0 0, 10 0, 10 10, 0 10, 0 0))"))
  expect_true(
    s2_iscollection("MULTIPOLYGON (
      ((40 40, 20 45, 45 30, 40 40)),
      ((20 35, 10 30, 10 10, 30 5, 45 20, 20 35), (30 20, 20 15, 20 25, 30 20))
    )")
  )
})

test_that("s2_dimension works", {
  expect_identical(s2_dimension(NA_character_), NA_integer_)
  expect_identical(s2_dimension("POINT EMPTY"), 0L)
  expect_identical(s2_dimension("LINESTRING EMPTY"), 1L)
  expect_identical(s2_dimension("POLYGON EMPTY"), 2L)
})

test_that("s2_numpoints works", {
  expect_identical(s2_numpoints(NA_character_), NA_integer_)
  expect_identical(s2_numpoints("POINT (-64 45)"), 1L)
  expect_identical(s2_numpoints("POINT EMPTY"), 0L)
  expect_identical(s2_numpoints("LINESTRING (0 0, 1 1)"), 2L)
  expect_identical(s2_numpoints("POLYGON ((0 0, 10 0, 10 10, 0 10, 0 0))"), 4L)
})

test_that("s2_isempty works", {
  expect_identical(s2_isempty(NA_character_), NA)
  expect_false(s2_isempty("POINT (-64 45)"))
  expect_true(s2_isempty("POINT EMPTY"))
  expect_false(s2_isempty("LINESTRING (0 0, 1 1)"))
  expect_true(s2_isempty("LINESTRING EMPTY"))
  expect_false(s2_isempty("POLYGON ((0 0, 10 0, 10 10, 0 10, 0 0))"))
  expect_true(s2_isempty("POLYGON EMPTY"))
})

test_that("s2_area works", {
  expect_identical(s2_area(NA_character_), NA_real_)
  expect_identical(s2_area("POINT (-64 45)"), 0)
  expect_identical(s2_area("POINT EMPTY"), 0)
  expect_identical(s2_area("LINESTRING (0 0, 1 1)"), 0)
  expect_identical(s2_area("POLYGON EMPTY"), 0)
  expect_identical(s2_area("POLYGON ((0 0, 90 0, 0 90, 0 0))", radius = 1), 4 * pi / 8)
  # make sure the radius is squared!
  expect_true(
    abs(s2_area("POLYGON ((0 0, 10 0, 10 10, 0 10, 0 0))", radius = 180 / pi) - 100) < 0.27
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
  expect_identical(s2_x("POINT EMPTY"), NA_real_)
  expect_identical(s2_y("POINT EMPTY"), NA_real_)
  expect_error(s2_x("LINESTRING EMPTY"), "Can't compute")
  expect_error(s2_y("LINESTRING EMPTY"), "Can't compute")
  expect_error(s2_x("POLYGON EMPTY"), "Can't compute")
  expect_error(s2_y("POLYGON EMPTY"), "Can't compute")
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

test_that("s2_maxdistance works", {
  expect_equal(
    s2_maxdistance("POINT (0 0)", "POINT (90 0)", radius = 180 / pi),
    90
  )

  expect_equal(
    s2_maxdistance("POINT (0 0)", "LINESTRING (90 0, 91 0)", radius = 180 / pi),
    91
  )

  expect_equal(
    s2_maxdistance("POINT (0 0)", "POLYGON ((90 0, 91 0, 89 1, 90 0))", radius = 180 / pi),
    91
  )

  expect_identical(s2_maxdistance("POINT (0 0)", NA_character_), NA_real_)
  expect_identical(s2_maxdistance(NA_character_, "POINT (0 0)"), NA_real_)
  expect_identical(s2_maxdistance("POINT (0 0)", "POINT EMPTY"), NA_real_)
  expect_identical(s2_maxdistance("POINT EMPTY", "POINT (0 0)"), NA_real_)
})
