
test_that("s2_geography class works", {
  geog <- new_s2_xptr(list(NULL), class = "s2_geography")
  expect_output(print(geog), "s2_geography")
  expect_identical(as_s2_geography(geog), geog)

  # subset assignment
  geog2 <- geog
  geog2[1] <- geog
  expect_identical(geog2, geog)

  geog2 <- geog
  geog2[[1]] <- geog
  expect_identical(geog2, geog)
})

test_that("s2_geography vectors can't have other types of objects concatenated or asssigned", {
  geog <- new_s2_xptr(list(NULL), class = "s2_geography")
  expect_is(c(geog, geog), "s2_geography")
  expect_error(c(geog, new_s2_xptr(list(), class = "some_other_class")), "All items must inherit")
  expect_error(geog[1] <- new_s2_xptr(list(NULL), class = "some_other_class"), "no applicable method")
  expect_error(geog[[1]] <- new_s2_xptr(list(NULL), class = "some_other_class"), "no applicable method")
})

test_that("s2_geography vectors can be created from s2_lnglat  and s2_point", {
  expect_wkt_equal(as_s2_geography(s2_lnglat(-64, 45)), "POINT (-64 45)")
  expect_wkt_equal(as_s2_geography(as_s2_point(s2_lnglat(-64, 45))), "POINT (-64 45)")
})

test_that("s2_geography vectors can be created from WKB and WKT", {
  wkb_point <- wk::as_wkb("POINT (-64 45)")
  expect_output(print(as_s2_geography(wkb_point)), "<POINT \\(-64 45\\)>")

  wkt_point <- wk::as_wkt("POINT (-64 45)")
  expect_output(print(as_s2_geography(wkt_point)), "<POINT \\(-64 45\\)>")

  # also test other classes commonly used to signify WKB or WKT
  expect_output(print(as_s2_geography(structure(wkb_point, class = "WKB")), "<POINT \\(-64 45\\)>"))
  expect_output(print(as_s2_geography(structure(wkb_point, class = "blob")), "<POINT \\(-64 45\\)>"))
})

test_that("s2_geography can be exported to WKB/WKT", {
  expect_identical(
    wk::as_wkt(wk::as_wkb(as_s2_geography("POINT (-64 45)")), precision = 10),
    wk::as_wkt(wk::as_wkb("POINT (-64 45)"), precision = 10)
  )
  expect_identical(
    wk::as_wkt(as_s2_geography("POINT (-64 45)"), precision = 10),
    wk::as_wkt("POINT (-64 45)")
  )
})

test_that("s2_geography vectors can be created from wkt", {
  expect_output(print(as_s2_geography("POINT (-64 45)")), "<POINT \\(-64 45\\)>")
  expect_output(print(as_s2_geography("POINT EMPTY")), "<POINT EMPTY>")
  expect_output(
    print(as_s2_geography("MULTIPOINT ((-64 45), (30 10))")),
    "<MULTIPOINT \\(\\(-64 45\\), \\(30 10\\)\\)>"
  )

  expect_output(
    print(as_s2_geography("LINESTRING (-64 45, 0 0)")),
    "<LINESTRING \\(-64 45, 0 0\\)>"
  )
  expect_output(
    print(as_s2_geography("LINESTRING EMPTY")),
    "<LINESTRING EMPTY>"
  )
  expect_output(
    print(as_s2_geography("MULTILINESTRING ((-64 45, 0 0), (0 1, 2 3))")),
    "<MULTILINESTRING \\(\\(-64 45, 0 0), \\(0 1, 2 3\\)\\)>"
  )

  expect_output(print(as_s2_geography("POLYGON EMPTY"), "<POLYGON EMPTY>"))
  expect_output(
    print(as_s2_geography("POLYGON ((0 0, 10 0, 10 10, 0 10, 0 0))")),
    "<POLYGON \\(\\(0 0, 10 0, 10 10"
  )
  expect_output(
    print(as_s2_geography("MULTIPOLYGON (((0 0, 10 0, 10 10, 0 10, 0 0)))")),
    "<POLYGON \\(\\(0 0, 10 0, 10 10"
  )
  expect_output(
    print(
      as_s2_geography("MULTIPOLYGON (
        ((40 40, 20 45, 45 30, 40 40)),
        ((20 35, 10 30, 10 10, 30 5, 45 20, 20 35), (30 20, 20 15, 20 25, 30 20))
      )")
    ),
    "<MULTIPOLYGON"
  )

  expect_output(
    print(as_s2_geography("GEOMETRYCOLLECTION (POINT (-64 45))")),
    "<GEOMETRYCOLLECTION \\(POINT \\(-64 45\\)\\)>"
  )

  expect_output(
    print(
      as_s2_geography(
        "GEOMETRYCOLLECTION (
          POINT (30 10),
          MULTIPOINT (11 12, 12 13),
          LINESTRING (40 40, 40 41),
          MULTILINESTRING ((-10 -12, -12 -13), (-15 -15, -16 -16)),
          POLYGON ((0 0, 10 0, 10 10, 0 10, 0 0)),
          MULTIPOLYGON (((0 0, -10 0, -10 -10, 0 -10, 0 0))),
          GEOMETRYCOLLECTION (POINT (-100 0), MULTIPOINT(-101 0, -102 0))
        )"
      ),
      max_coords = 100
    ),
    paste0(
      "GEOMETRYCOLLECTION.*?POINT.*?MULTIPOINT.*?LINESTRING.*?MULTILINESTRING.*?",
      "POLYGON.*?POLYGON.*?GEOMETRYCOLLECTION.*?POINT.*?MULTIPOINT"
    )
  )

  expect_output(print(as_s2_geography("GEOMETRYCOLLECTION EMPTY")), "<GEOMETRYCOLLECTION EMPTY>")
})

test_that("nested ring depths are correctly exported", {
  # polygon with hole
  expect_output(
    print(
      as_s2_geography("MULTIPOLYGON (
        ((40 40, 20 45, 45 30, 40 40)),
        (
          (20 35, 10 30, 10 10, 30 5, 45 20, 20 35),
          (30 20, 20 15, 20 25, 30 20)
        )
      )"),
      max_coords = 100
    ),
    "\\(20 35, 10 30, 10 10, 30 5, 45 20, 20 35\\), \\(30 20, 20 15, 20 25, 30 20"
  )

  # polygon with a hole in a hole!
  expect_output(
    print(
      as_s2_geography("MULTIPOLYGON (
        ((40 40, 20 45, 45 30, 40 40)),
        (
          (20 35, 10 30, 10 10, 30 5, 45 20, 20 35),
          (30 20, 20 15, 20 25, 30 20),
          (27 21, 21 21, 21 16, 27 21)
        )
      )"),
      max_coords = 100
    ),
    "30 20, 20 15, 20 25, 30 20\\), \\(27 21, 21 21, 21 16, 27 21"
  )
})

test_that("polygons with holes are interpreted as such by S2", {
  expect_true(
    s2_intersects(
      "MULTIPOLYGON (
        ((40 40, 20 45, 45 30, 40 40)),
        (
          (20 35, 10 30, 10 10, 30 5, 45 20, 20 35),
          (30 20, 20 15, 20 25, 30 20),
          (27 21, 21 21, 21 16, 27 21)
        )
      )",
      "POINT (23 19.5)"
    )
  )

  expect_false(
    s2_intersects(
      "MULTIPOLYGON (
        ((40 40, 20 45, 45 30, 40 40)),
        (
          (20 35, 10 30, 10 10, 30 5, 45 20, 20 35),
          (30 20, 20 15, 20 25, 30 20)
        )
      )",
      "POINT (23 19.5)"
    )
  )
})

test_that("polygon construction works with oriented = TRUE and oriented = FALSE", {
  polygon_with_bad_hole_nested <- as_s2_geography("MULTIPOLYGON (
    ((40 40, 20 45, 45 30, 40 40)),
    (
      (20 35, 10 30, 10 10, 30 5, 45 20, 20 35),
      (30 20, 20 25, 20 15, 30 20)
    )
  )", oriented = FALSE)

  expect_false(s2_intersects(polygon_with_bad_hole_nested, "POINT (23 19.5)"))

  expect_error(
    as_s2_geography("MULTIPOLYGON (
      ((40 40, 20 45, 45 30, 40 40)),
      (
        (20 35, 10 30, 10 10, 30 5, 45 20, 20 35),
        (30 20, 20 25, 20 15, 30 20)
      )
    )", oriented = TRUE, check = TRUE),
    "Inconsistent loop orientations"
  )

  expect_silent(
    as_s2_geography("MULTIPOLYGON (
      ((40 40, 20 45, 45 30, 40 40)),
      (
        (20 35, 10 30, 10 10, 30 5, 45 20, 20 35),
        (30 20, 20 25, 20 15, 30 20)
      )
    )", oriented = TRUE, check = FALSE)
  )
})

test_that("Full polygons work", {
  expect_true(s2_intersects(as_s2_geography(TRUE), "POINT(0 1)"))
  expect_wkt_equal(s2_difference(as_s2_geography(TRUE), "POINT(0 1)"), "POLYGON ((0 -90, 0 -90))")
})
