
test_that("s2_geog_point() works", {
  expect_wkt_equal(s2_geog_point(-64, 45), "POINT (-64 45)")
})

test_that("s2_make_line() works", {
  expect_wkt_equal(
    s2_make_line(c(-64, 8), c(45, 71)),
    "LINESTRING (-64 45, 8 71)"
  )

  # check separation using feature_id
  expect_wkt_equal(
    s2_make_line(
      c(-64, 8, -27, -27), c(45, 71, 0, 45),
      feature_id = c(1, 1, 2, 2)
    ),
    c("LINESTRING (-64 45, 8 71)", "LINESTRING (-27 0, -27 45)")
  )
})

test_that("s2_make_polygon() works", {
  expect_wkt_equal(
    s2_make_polygon(c(-45, 8, 0), c(64, 71, 90)),
    "POLYGON ((-45 64, 8 71, 0 90, -45 64))"
  )
  # check that loops can be open or closed
  expect_wkt_equal(
    s2_make_polygon(c(-45, 8, 0, -45), c(64, 71, 90, 64)),
    "POLYGON ((-45 64, 8 71, 0 90, -45 64))"
  )

  # check feature/ring separation
  expect_wkt_equal(
    s2_make_polygon(
      c(20, 10, 10, 30, 45, 30, 20, 20, 40, 20, 45),
      c(35, 30, 10, 5, 20, 20, 15, 25, 40, 45, 30),
      feature_id = c(rep(1, 8), rep(2, 3)),
      ring_id = c(1, 1, 1, 1, 1, 2, 2, 2, 1, 1, 1)
    ),
    c(
      "POLYGON ((20 35, 10 30, 10 10, 30 5, 45 20, 20 35), (30 20, 20 15, 20 25, 30 20))",
      "POLYGON ((40 40, 20 45, 45 30, 40 40))"
    )
  )
})

test_that("s2_geog_from_wkt() works", {
  expect_wkt_equal(s2_geog_from_text("POINT (-64 45)"), "POINT (-64 45)")
})

test_that("s2_geog_from_wkb() works", {
  expect_wkt_equal(s2_geog_from_wkb(as_wkb("POINT (-64 45)")), "POINT (-64 45)")
})

test_that("s2_as_text() works", {
  expect_identical(
    s2_as_text("POINT (0.1234567890123456 0.1234567890123456)"),
    "POINT (0.1234567890123456 0.1234567890123456)"
  )
})

test_that("s2_as_binary() works", {
  expect_identical(
    s2_as_binary("POINT (0 0)", endian = 1),
    structure(
      list(
        as.raw(
          c(0x01, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00,
            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
            0x00, 0x00, 0x00
          )
        )
      ),
      class = "blob"
    )
  )
})

test_that("s2_as_binary works on (multi)polygons", {
	geog <- s2_data_countries()
	wkb <- s2_as_binary(geog)

	expect_identical(
	  sum(vapply(wkb, length, integer(1))),
	  173318L
	)
	expect_identical(length(wkb), length(geog))
})

test_that("polygon constructors respect oriented and check arguments", {
  polygon_with_bad_hole_wkt <- "POLYGON (
    (20 35, 10 30, 10 10, 30 5, 45 20, 20 35),
    (30 20, 20 25, 20 15, 30 20)
  )"
  polygon_with_bad_hole_wkb <- wk::as_wkb(polygon_with_bad_hole_wkt)
  polygon_with_bad_hole_df <- data.frame(
    x = c(20, 10, 10, 30, 45, 30, 20, 20),
    y = c(35, 30, 10, 5, 20, 20, 25, 15),
    ring_id = c(1, 1, 1, 1, 1, 2, 2, 2)
  )

  expect_false(
    s2_intersects(
      s2_geog_from_text(polygon_with_bad_hole_wkt, oriented = FALSE),
      "POINT (23 19.5)"
    )
  )
  expect_false(
    s2_intersects(
      s2_geog_from_wkb(polygon_with_bad_hole_wkb, oriented = FALSE),
      "POINT (23 19.5)"
    )
  )
  expect_false(
    s2_intersects(
      with(
        polygon_with_bad_hole_df,
        s2_make_polygon(x, y, ring_id = ring_id, oriented = FALSE)
      ),
      "POINT (23 19.5)"
    )
  )

  expect_error(
    s2_intersects(
      s2_geog_from_text(polygon_with_bad_hole_wkt, oriented = TRUE, check = TRUE),
      "POINT (23 19.5)"
    ),
    "Inconsistent loop orientations"
  )
  expect_error(
    s2_intersects(
      s2_geog_from_wkb(polygon_with_bad_hole_wkb, oriented = TRUE, check = TRUE),
      "POINT (23 19.5)"
    ),
    "Inconsistent loop orientations"
  )
  expect_error(
    s2_intersects(
      with(
        polygon_with_bad_hole_df,
        s2_make_polygon(x, y, ring_id = ring_id, oriented = TRUE, check = TRUE)
      ),
      "POINT (23 19.5)"
    ),
    "Inconsistent loop orientations"
  )

  expect_silent(
    s2_intersects(
      s2_geog_from_text(polygon_with_bad_hole_wkt, oriented = TRUE, check = FALSE),
      "POINT (23 19.5)"
    )
  )
  expect_silent(
    s2_intersects(
      s2_geog_from_wkb(polygon_with_bad_hole_wkb, oriented = TRUE, check = FALSE),
      "POINT (23 19.5)"
    )
  )
  expect_silent(
    s2_intersects(
      with(
        polygon_with_bad_hole_df,
        s2_make_polygon(x, y, ring_id = ring_id, oriented = TRUE, check = FALSE)
      ),
      "POINT (23 19.5)"
    )
  )
})
