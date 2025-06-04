# Serialization routines
test_that("s2_geography_serialize() and s2_geography_deserialize() work", {
  g <- s2_geog_from_text("POLYGON ((0 0, 10 0, 10 10, 0 10, 0 0))")

  expect_wkt_equal(
    s2_geography_unserialize(s2_geography_serialize(g)),
    g
  )
})

test_that("Serialization does not lose precision", {
  # this polygons fails to correctly serialize using s2_as_binary()
  g <- s2_make_polygon(
    c(
      180,  180,
      179.364142661964, 178.725059362997,
      178.596838595117, 179.096609362997,
      179.413509362997, 180
    ),
    c(-16.0671326636424, -16.5552165666392,
      -16.8013540769469, -17.012041674368,
      -16.63915, -16.4339842775474,
      -16.3790542775474, -16.0671326636424
    )
  )

  expect_wkt_equal(
    s2_geography_unserialize(s2_geography_serialize(g)),
    g
  )
})

# S2 Geography constructors
test_that("s2_geography() can be correctly serialized", {
  expect_wkt_serializeable(s2_geography())
})

test_that("s2_geog_point() can be correctly serialized", {
  expect_wkt_serializeable(s2_geog_point(
    -64, 45
  ))
})

test_that("s2_make_line() can be correctly serialized", {
  expect_wkt_serializeable(s2_make_line(
    c(-64, 8), c(45, 71)
  ))
})

test_that("s2_make_polygon() can be correctly serialized", {
  expect_wkt_serializeable(s2_make_polygon(
    c(-45, 8, 0), c(64, 71, 90)
  ))
  expect_wkt_serializeable(s2_make_polygon(
    c(-45, 8, 0, -45), c(64, 71, 90, 64)
  ))
})

test_that("s2_geog_from_wkt() can be correctly serialized", {
  expect_wkt_serializeable(s2_geog_from_text(
    "POINT (-64 45)"
  ))
})

test_that("s2_geog_from_wkb() can be correctly serialized", {
  expect_wkt_serializeable(s2_geog_from_wkb(
    as_wkb("POINT (-64 45)")
  ))
})

# Geography Transformations
test_that("s2_union() can be correctly serialized", {
  expect_wkt_serializeable(s2_union(
    "POINT (10 30)",
    "POINT (30 10)"
  ))
})

test_that("s2_intersection() can be correctly serialized", {
  expect_wkt_serializeable(s2_intersection(
    "POLYGON ((0 0, 10 0, 10 10, 0 10, 0 0))",
    "POLYGON ((5 5, 15 5, 15 15, 5 15, 5 5))",
  ))
})


test_that("s2_difference() can be correctly serialized", {
  expect_wkt_serializeable(s2_difference(
    "POLYGON ((0 0, 10 0, 10 10, 0 10, 0 0))",
    "POLYGON ((5 5, 15 5, 15 15, 5 15, 5 5))",
  ))
})

test_that("s2_sym_difference() can be correctly serialized", {
  expect_wkt_serializeable(s2_sym_difference(
    "POLYGON ((0 0, 10 0, 10 10, 0 10, 0 0))",
    "POLYGON ((5 5, 15 5, 15 15, 5 15, 5 5))",
  ))
})

test_that("s2_convex_hull() can be correctly serialized", {
  expect_wkt_serializeable(s2_convex_hull(
    "GEOMETRYCOLLECTION (POINT (-1 0), POINT (0 1), POINT (1 0))"
  ))
})

test_that("s2_boundary() can be correctly serialized", {
  expect_wkt_serializeable(s2_boundary(
    "POLYGON ((0 0, 10 0, 10 10, 0 10, 0 0))"
  ))
})


test_that("s2_centroid() can be correctly serialized", {
  expect_wkt_serializeable(s2_centroid(
    "POLYGON ((0 0, 10 0, 10 10, 0 10, 0 0))"
  ))
})

test_that("s2_closest_point() can be correctly serialized", {
  expect_wkt_serializeable(s2_closest_point(
    "POLYGON ((0 0, 10 0, 10 10, 0 10, 0 0))",
    "POINT (-63 46)"
  ))
})

test_that("s2_minimum_clearance_line_between() can be correctly serialized", {
  expect_wkt_serializeable(s2_minimum_clearance_line_between(
    "POINT (10 30)",
    "POINT (30 10)"
  ))
})

test_that("s2_snap_to_grid() can be correctly serialized", {
  expect_wkt_serializeable(s2_snap_to_grid(
    "POINT (10.25 30.5)",
    1
  ))
})

test_that("s2_simplify() can be correctly serialized", {
  expect_wkt_serializeable(s2_simplify(
    "POLYGON ((0 0, 10 0, 10 10, 0 10, 0 0))",
    1
  ))
})

test_that("s2_rebuild() can be correctly serialized", {
  expect_wkt_serializeable(s2_rebuild(
    "POLYGON ((0 0, 10 0, 10 10, 0 10, 0 0))"
  ))
})

test_that("s2_buffer_cells() can be correctly serialized", {
  expect_wkt_serializeable(s2_buffer_cells(
    "POINT (10 10)", 100
  ))
})

test_that("s2_centroid_agg() can be correctly serialized", {
  expect_wkt_serializeable(s2_centroid_agg(c(
    "POINT (-1 0)",
    "POINT (0 1)",
    "POINT (1 0)"
  )))
})

test_that("s2_coverage_union_agg() can be correctly serialized", {
  expect_wkt_serializeable(s2_coverage_union_agg(c(
    "POINT (-1 0)",
    "POINT (0 1)",
    "POINT (1 0)"
  )))
})

test_that("s2_rebuild_agg() can be correctly serialized", {
  expect_wkt_serializeable(s2_rebuild_agg(c(
    "POINT (-1 0)",
    "POINT (0 1)",
    "POINT (1 0)"
  )))
})

test_that("s2_union_agg() can be correctly serialized", {
  expect_wkt_serializeable(s2_union_agg(c(
    "POINT (-1 0)",
    "POINT (0 1)",
    "POINT (1 0)"
  )))
})

test_that("s2_convex_hull_agg() can be correctly serialized", {
  expect_wkt_serializeable(s2_convex_hull_agg(c(
    "POINT (-1 0)",
    "POINT (0 1)",
    "POINT (1 0)"
  )))
})

test_that("s2_point_on_surface() can be correctly serialized", {
  expect_wkt_serializeable(s2_point_on_surface(
    "POLYGON ((0 0, 10 0, 10 10, 0 10, 0 0))"
  ))
})

# S2 cell operators that construct s2 geography
test_that("s2_cell_center() can be correctly serialized", {
  expect_wkt_serializeable(s2_cell_center(s2_cell("5")))
})

test_that("s2_cell_boundary() can be correctly serialized", {
  expect_wkt_serializeable(s2_cell_boundary(s2_cell("5")))
})

test_that("s2_cell_polygon() can be correctly serialized", {
  expect_wkt_serializeable(s2_cell_polygon(s2_cell("5")))
})

test_that("s2_cell_vertex() can be correctly serialized", {
  expect_wkt_serializeable(s2_cell_vertex(s2_cell("5"), seq_len(4L)))
})
