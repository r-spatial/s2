
test_that("s2_centroid() works", {
  expect_wkt_equal(s2_centroid("POINT (30 10)"), "POINT (30 10)")
  expect_true(s2_is_empty(s2_centroid("POINT EMPTY")))
  expect_wkt_equal(s2_centroid("MULTIPOINT ((0 0), (0 10))"), "POINT (0 5)")
  expect_wkt_equal(s2_centroid("LINESTRING (0 0, 0 10)"), "POINT (0 5)", precision = 15)
  expect_near(
    s2_distance(
      s2_centroid("POLYGON ((-5 -5, 5 -5, 5 5, -5 5, -5 -5))"),
      "POINT (0 0)"
    ),
    0,
    epsilon = 1e-6
  )
})

test_that("s2_boundary() works", {
  expect_true(s2_is_empty(s2_boundary("POINT (30 10)")))
  expect_true(s2_is_empty(s2_boundary("POINT EMPTY")))
  expect_true(s2_is_empty(s2_boundary("POLYGON EMPTY")))
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

test_that("s2_closest_point() works", {
  expect_wkt_equal(s2_closest_point("POINT (0 1)", "POINT (30 10)"), "POINT (0 1)")
  expect_wkt_equal(s2_closest_point("LINESTRING (0 1, -12 -12)", "POINT (30 10)"), "POINT (0 1)")
})

test_that("s2_minimum_clearance_line_between() works", {
  expect_wkt_equal(
    s2_minimum_clearance_line_between("POINT (0 1)", "POINT (30 10)"),
    "LINESTRING (0 1, 30 10)"
  )
  expect_true(s2_is_empty(s2_minimum_clearance_line_between("POINT (30 10)", "POINT EMPTY")))

  expect_wkt_equal(
    s2_minimum_clearance_line_between("LINESTRING (0 1, -12 -12)", "POINT (30 10)"),
    "LINESTRING (0 1, 30 10)"
  )
  expect_wkt_equal(
    s2_minimum_clearance_line_between("LINESTRING (0 0, 1 1)", "LINESTRING (1 0, 0 1)"),
    "MULTIPOINT ((0.5 0.500057), (0.5 0.500057))",
  	precision = 6
  )
})

test_that("s2_difference() works", {
  expect_wkt_equal(s2_difference("POINT (30 10)", "POINT EMPTY"), "POINT (30 10)")
  expect_true(s2_is_empty(s2_difference("POINT (30 10)", "POINT (30 10)")))

  expect_true(s2_is_empty(s2_difference("LINESTRING (0 0, 45 0)", "LINESTRING (0 0, 45 0)")))
})

test_that("s2_difference() works for polygons", {
  # on Windows i386, these fail without snap rounding
  df <- s2_difference(
    "POLYGON ((0 0, 10 0, 10 10, 0 10, 0 0))",
    "POLYGON ((5 5, 15 5, 15 15, 5 15, 5 5))",
    s2_options(snap = s2_snap_level(30))
  )

  expect_near(
    s2_area(df, radius = 1),
    s2_area("POLYGON ((0 0, 10 0, 10 10, 0 10, 0 0))", radius = 1) -
      s2_area("POLYGON ((5 5, 10 5, 10 15, 5 10, 5 5))", radius = 1),
    epsilon = 0.004
  )

  df0 <- s2_difference(
    "POLYGON ((0 0, 10 0, 10 10, 0 10, 0 0))",
    "POLYGON ((5 5, 15 5, 15 15, 5 15, 5 5))" ,
    s2_options(model = 0, snap = s2_snap_level(30))
  )
  df1 <- s2_difference(
    "POLYGON ((0 0, 10 0, 10 10, 0 10, 0 0))",
    "POLYGON ((5 5, 15 5, 15 15, 5 15, 5 5))" ,
    s2_options(model = 1, snap = s2_snap_level(30))
  )
  df2 <- s2_difference(
    "POLYGON ((0 0, 10 0, 10 10, 0 10, 0 0))",
    "POLYGON ((5 5, 15 5, 15 15, 5 15, 5 5))" ,
    s2_options(model = 2, snap = s2_snap_level(30))
  )
  expect_equal(s2_area(df0) - s2_area(df2), 0.0)
  expect_equal(s2_area(df0) - s2_area(df1), 0.0)
})

test_that("s2_sym_difference() works", {
  expect_wkt_equal(s2_sym_difference("POINT (30 10)", "POINT EMPTY"), "POINT (30 10)")
  expect_true(s2_is_empty(s2_sym_difference("POINT (30 10)", "POINT (30 10)")))
  expect_wkt_equal(s2_sym_difference("POINT (30 10)", "POINT (30 20)"), "MULTIPOINT ((30 20), (30 10))")

  expect_true(s2_is_empty(s2_sym_difference("LINESTRING (0 0, 45 0)", "LINESTRING (0 0, 45 0)")))
})

test_that("s2_sym_difference() works for polygons", {
  # on Windows i386, these fail without snap rounding
  df <- s2_sym_difference(
    "POLYGON ((0 0, 10 0, 10 10, 0 10, 0 0))",
    "POLYGON ((5 5, 15 5, 15 15, 5 15, 5 5))",
    s2_options(snap = s2_snap_level(30))
  )

  expect_near(
    s2_area(df, radius = 1),
    2 * s2_area("POLYGON ((0 0, 10 0, 10 10, 0 10, 0 0))", radius = 1) -
      s2_area("POLYGON ((5 5, 10 5, 10 15, 5 10, 5 5))", radius = 1),
    epsilon = 0.0042
  )

  df0 <- s2_sym_difference(
    "POLYGON ((0 0, 10 0, 10 10, 0 10, 0 0))",
    "POLYGON ((5 5, 15 5, 15 15, 5 15, 5 5))" ,
    s2_options(model = 0, snap = s2_snap_level(30))
  )

  df1 <- s2_sym_difference(
    "POLYGON ((0 0, 10 0, 10 10, 0 10, 0 0))",
    "POLYGON ((5 5, 15 5, 15 15, 5 15, 5 5))" ,
    s2_options(model = 1, snap = s2_snap_level(30))
  )

  df2 = s2_sym_difference(
    "POLYGON ((0 0, 10 0, 10 10, 0 10, 0 0))",
    "POLYGON ((5 5, 15 5, 15 15, 5 15, 5 5))",
    s2_options(model = 2, snap = s2_snap_level(30))
  )
  expect_equal(s2_area(df0) - s2_area(df2), 0.0)
  expect_equal(s2_area(df0) - s2_area(df1), 0.0)
})

test_that("s2_intersection() works", {
  expect_wkt_equal(s2_intersection("POINT (30 10)", "POINT (30 10)"), "POINT (30 10)")
  expect_true(s2_is_empty(s2_intersection("POINT (30 10)", "POINT (30 11)")))

  expect_wkt_equal(
    s2_intersection(
      "POLYGON ((0 0, 10 0, 10 10, 0 10, 0 0))",
      "LINESTRING (0 5, 10 5)"
    ),
    "LINESTRING (0 5, 10 5)"
  )

  expect_equal(
    s2_distance(
      s2_intersection("LINESTRING (-45 0, 45 0)", "LINESTRING (0 -10, 0 10)"),
      "POINT (0 0)"
    ),
    0
  )
})

test_that("s2_intersction() works for polygons", {
  expect_wkt_equal(
    s2_intersection(
      "POLYGON ((0 0, 10 0, 10 10, 0 10, 0 0))",
      "POLYGON ((5 5, 15 5, 15 15, 5 15, 5 5))",
      s2_options(snap = s2_snap_level(30))
    ),
    "POLYGON ((5 5, 10 5, 10 10, 5 10, 5 5))",
    precision = 2
  )
  expect_true(s2_is_empty(s2_intersection("POLYGON ((0 0, 10 0, 10 10, 0 10, 0 0))", "POINT(0 0)")))
  expect_true(
    s2_is_empty(
      s2_intersection(
        "POLYGON ((0 0, 10 0, 10 10, 0 10, 0 0))", "POINT(0 0)", s2_options(model = 0)
      )
    )
  )
  expect_true(
    s2_is_empty(
      s2_intersection("POLYGON ((0 0, 10 0, 10 10, 0 10, 0 0))", "POINT(0 0)", s2_options(model = 1))
    )
  )
  expect_wkt_equal(
    s2_intersection("POLYGON ((0 0, 10 0, 10 10, 0 10, 0 0))", "POINT(0 0)", s2_options(model = 2)),
    "POINT(0 0)"
  )

  df0 <- s2_intersection(
    "POLYGON ((0 0, 10 0, 10 10, 0 10, 0 0))",
    "POLYGON ((5 5, 15 5, 15 15, 5 15, 5 5))" ,
    s2_options(model = 0)
  )
  df1 <- s2_intersection(
    "POLYGON ((0 0, 10 0, 10 10, 0 10, 0 0))",
    "POLYGON ((5 5, 15 5, 15 15, 5 15, 5 5))" ,
    s2_options(model = 1)
  )
  df2 <- s2_intersection(
    "POLYGON ((0 0, 10 0, 10 10, 0 10, 0 0))",
    "POLYGON ((5 5, 15 5, 15 15, 5 15, 5 5))" ,
    s2_options(model = 2)
  )

  expect_equal(s2_area(df0) - s2_area(df2), 0.0)
  expect_equal(s2_area(df0) - s2_area(df1), 0.0)
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
})

test_that("s2_union() works for polygons", {
  # on Windows i386, these fail without snap rounding
  u <- s2_union(
    "POLYGON ((0 0, 10 0, 10 10, 0 10, 0 0))",
    "POLYGON ((5 5, 15 5, 15 15, 5 15, 5 5))",
    s2_options(snap = s2_snap_level(30))
  )
  u0 <- s2_union(
    "POLYGON ((0 0, 10 0, 10 10, 0 10, 0 0))",
    "POLYGON ((5 5, 15 5, 15 15, 5 15, 5 5))" ,
    s2_options(model = 0, snap = s2_snap_level(30))
  )
  u1 <- s2_union(
    "POLYGON ((0 0, 10 0, 10 10, 0 10, 0 0))",
    "POLYGON ((5 5, 15 5, 15 15, 5 15, 5 5))" ,
    s2_options(model = 1, snap = s2_snap_level(30))
  )
  u2 <- s2_union(
    "POLYGON ((0 0, 10 0, 10 10, 0 10, 0 0))",
    "POLYGON ((5 5, 15 5, 15 15, 5 15, 5 5))" ,
    s2_options(model = 2, snap = s2_snap_level(30))
  )
  expect_equal(s2_area(u0) - s2_area(u2), 0.0)
  expect_equal(s2_area(u0) - s2_area(u1), 0.0)


  expect_near(
    s2_area(u, radius = 1),
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
    as_s2_geography(NA_character_)
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
    as_s2_geography(NA_character_)
  )
  expect_wkt_equal(
    s2_centroid_agg(c("POINT (30 10)", NA), na.rm = TRUE),
    "POINT (30 10)"
  )
})

test_that("s2_snap_to_grid() works", {
  expect_wkt_equal(
    s2_as_text(s2_snap_to_grid("POINT (0.333333333333 0.666666666666)", 1e-2)),
    "POINT (0.33 0.67)",
    precision = 6
  )
})

test_that("s2_buffer() works", {
  ply <- s2_buffer_cells("POINT (0 0)", distance = 1, radius = 180 / pi)
  box <- wk::wkb_ranges(wk::as_wkb(ply))
  expect_near(box$xmin, -1, epsilon = 0.002)
  expect_near(box$ymin, -1, epsilon = 0.002)
  expect_near(box$xmax, 1, epsilon = 0.002)
  expect_near(box$ymax, 1, epsilon = 0.002)
})

test_that("real data survives the S2BooleanOperation", {
  for (continent in unique(s2::s2_data_tbl_countries$continent)) {
    # this is primarily a test of the S2BooleanOperation -> Geography constructor
    unioned <- expect_is(s2_union_agg(s2_data_countries(continent)), "s2_geography")

    # this is a test of Geography::Export() on potentially complex polygons
    exported <- expect_length(s2_as_binary(unioned), 1)

    # the output WKB should load as a polygon with oriented = TRUE and result in the
    # same number of points and similar area
    reloaded <- as_s2_geography(structure(exported, class = "wk_wkb"), oriented = TRUE)
    expect_equal(s2_num_points(reloaded), s2_num_points(unioned))
    expect_equal(s2_area(reloaded, radius = 1), s2_area(unioned, radius = 1))

    # also check with oriented = FALSE (may catch quirky nesting)
    reloaded <- as_s2_geography(structure(exported, class = "wk_wkb"), oriented = FALSE)
    expect_equal(s2_num_points(reloaded), s2_num_points(unioned))
    expect_equal(s2_area(reloaded, radius = 1), s2_area(unioned, radius = 1))
  }
})
