
test_that("s2_centroid() works", {
  expect_wkt_equal(s2_centroid("POINT (30 10)"), "POINT (30 10)")
  expect_true(s2_isempty(s2_centroid("POINT EMPTY")))
  expect_wkt_equal(s2_centroid("MULTIPOINT ((0 0), (0 10))"), "POINT (0 5)")
  expect_wkt_equal(s2_centroid("LINESTRING (0 0, 0 10)"), "POINT (0 5)", precision = 15)
  expect_wkt_equal(s2_centroid("POLYGON ((-5 -5, 5 -5, 5 5, -5 5, -5 -5))"), "POINT (0 0)", precision = 10)
})

test_that("s2_boundary() works", {
  expect_true(s2_isempty(s2_boundary("POINT (30 10)")))
  expect_true(s2_isempty(s2_boundary("POINT EMPTY")))
  expect_true(s2_isempty(s2_boundary("POLYGON EMPTY")))
  expect_wkt_equal(s2_boundary("LINESTRING (0 0, 0 10)"), "MULTIPOINT ((0 0), (0 10))")

  expect_wkt_equal(
    s2_boundary("MULTIPOLYGON (
        ((40 40, 20 45, 45 30, 40 40)),
        ((20 35, 10 30, 10 10, 30 5, 45 20, 20 35), (30 20, 20 15, 20 25, 30 20))
    )"),
    "MULTILINESTRING (
        (40 40, 20 45, 45 30, 40 40),
        (20 35, 10 30, 10 10, 30 5, 45 20, 20 35), (30 20, 20 15, 20 25, 30 20)
    )",
    precision = 15
  )
})

test_that("s2_closestpoint() works", {
  expect_wkt_equal(s2_closestpoint("POINT (0 1)", "POINT (30 10)"), "POINT (0 1)")
  expect_true(s2_isempty(s2_closestpoint("POINT (30 10)", "POINT EMPTY")))

  expect_wkt_equal(s2_closestpoint("LINESTRING (0 1, -12 -12)", "POINT (30 10)"), "POINT (0 1)")
})

test_that("s2_difference() works", {
  expect_wkt_equal(s2_difference("POINT (30 10)", "POINT EMPTY"), "POINT (30 10)")
  expect_true(s2_isempty(s2_difference("POINT (30 10)", "POINT (30 10)")))

  expect_true(s2_isempty(s2_difference("LINESTRING (0 0, 45 0)", "LINESTRING (0 0, 45 0)")))

  skip("this fails on Windows (probably needs some degree of snap rounding)")
  expect_near(
    s2_area(
      s2_difference(
        "POLYGON ((0 0, 10 0, 10 10, 0 10, 0 0))",
        "POLYGON ((5 5, 15 5, 15 15, 5 15, 5 5))"
      ),
      radius = 1
    ),
    s2_area("POLYGON ((0 0, 10 0, 10 10, 0 10, 0 0))", radius = 1) -
      s2_area("POLYGON ((5 5, 10 5, 10 15, 5 10, 5 5))", radius = 1),
    epsilon = 0.004
  )
})

test_that("s2_intersection() works", {
  expect_wkt_equal(s2_intersection("POINT (30 10)", "POINT (30 10)"), "POINT (30 10)")
  expect_true(s2_isempty(s2_intersection("POINT (30 10)", "POINT (30 11)")))

  expect_wkt_equal(
    s2_intersection(
      "POLYGON ((0 0, 10 0, 10 10, 0 10, 0 0))",
      "POLYGON ((5 5, 15 5, 15 15, 5 15, 5 5))"
    ),
    "POLYGON ((5 5, 10 5, 10 10, 5 10, 5 5))",
    precision = 2
  )

  expect_wkt_equal(
    s2_intersection(
      "POLYGON ((0 0, 10 0, 10 10, 0 10, 0 0))",
      "LINESTRING (0 5, 10 5)"
    ),
    "LINESTRING (0 5, 10 5)"
  )

  skip("Don't know why this intersection fails (works on bigquery)")
  expect_wkt_equal(
    s2_intersection("LINESTRING (-45 0, 45 0)", "LINESTRING (0 -10, 0 10)"),
    "POINT (0 0)"
  )
})

test_that("s2_union(x) works", {
  expect_wkt_equal(s2_union("POINT (30 10)"), "POINT (30 10)")
  expect_wkt_equal(s2_union("POINT EMPTY"), "GEOMETRYCOLLECTION EMPTY")
  expect_wkt_equal(
    s2_union("MULTILINESTRING ((-45 0, 0 0), (0 0, 0 10))"),
    "LINESTRING (-45 0, 0 0, 0 10)"
  )

  expect_wkt_equal(s2_union("GEOMETRYCOLLECTION (POINT (30 10))"), "POINT (30 10)")
  expect_wkt_equal(
    s2_union("GEOMETRYCOLLECTION (POINT (30 10), LINESTRING (0 0, 0 1))"),
    "GEOMETRYCOLLECTION (POINT (30 10), LINESTRING (0 0, 0 1))"
  )
})

test_that("s2_union(x, y) works", {
  expect_wkt_equal(s2_union("POINT (30 10)", "POINT EMPTY"), "POINT (30 10)")
  expect_wkt_equal(s2_union("POINT EMPTY", "POINT EMPTY"), "GEOMETRYCOLLECTION EMPTY")

  # LINESTRING (-45 0, 0 0, 0 10)
  expect_wkt_equal(
    s2_union("LINESTRING (-45 0, 0 0)", "LINESTRING (0 0, 0 10)"),
    "LINESTRING (-45 0, 0 0, 0 10)"
  )

  skip("this fails on Windows (probably needs some degree of snap rounding)")
  expect_near(
    s2_area(
      s2_union(
        "POLYGON ((0 0, 10 0, 10 10, 0 10, 0 0))",
        "POLYGON ((5 5, 15 5, 15 15, 5 15, 5 5))"
      ),
      radius = 1
    ),
    s2_area("POLYGON ((0 0, 10 0, 10 10, 0 10, 0 0))", radius = 1) +
      s2_area("POLYGON ((5 5, 15 5, 15 15, 5 15, 5 5))", radius = 1) -
      s2_area("POLYGON ((5 5, 10 5, 10 15, 5 10, 5 5))", radius = 1),
    epsilon = 0.004
  )
})

test_that("s2_union_agg() works", {
  expect_wkt_equal(s2_union_agg(c("POINT (30 10)", "POINT EMPTY")), "POINT (30 10)")
  expect_wkt_equal(s2_union_agg(c("POINT EMPTY", "POINT EMPTY")), "GEOMETRYCOLLECTION EMPTY")

  # NULL handling
  expect_identical(
    s2_union_agg(c("POINT (30 10)", NA), na.rm = FALSE),
    s2geography(NA_character_)
  )
  expect_wkt_equal(
    s2_union_agg(c("POINT (30 10)", NA), na.rm = TRUE),
    "POINT (30 10)"
  )
})

test_that("s2_centroid_agg() works", {
  expect_wkt_equal(s2_centroid_agg(c("POINT (30 10)", "POINT EMPTY")), "POINT (30 10)")
  expect_wkt_equal(s2_centroid_agg(c("POINT EMPTY", "POINT EMPTY")), "POINT EMPTY")
  expect_wkt_equal(s2_centroid_agg(c("POINT (0 0)", "POINT (0 10)")), "POINT (0 5)", precision = 15)

  # NULL handling
  expect_identical(
    s2_centroid_agg(c("POINT (30 10)", NA), na.rm = FALSE),
    s2geography(NA_character_)
  )
  expect_wkt_equal(
    s2_centroid_agg(c("POINT (30 10)", NA), na.rm = TRUE),
    "POINT (30 10)"
  )
})
