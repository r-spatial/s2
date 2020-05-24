
test_that("s2geography class works", {
  geog <- new_s2xptr(list(NULL), class = "s2geography")
  expect_output(print(geog), "s2geography")
  expect_identical(s2geography(geog), geog)
})

test_that("s2geography vectors can be created from wkb points", {
  wkb_point <- list(as.raw(c(0x01, 0x01, 0x00, 0x00, 0x20, 0xe6, 0x10, 0x00,
                             0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x50, 0xc0, 0x00, 0x00,
                             0x00, 0x00, 0x00, 0x80, 0x46, 0x40)))
  class(wkb_point) <- "wk_wkb"

  expect_output(print(s2geography(wkb_point)), "<POINT \\(-64 45\\)>")
})

test_that("s2geography vectors can be created from wkt", {
  expect_output(print(s2geography("POINT (-64 45)")), "<POINT \\(-64 45\\)>")
  expect_output(print(s2geography("POINT EMPTY")), "<POINT EMPTY>")
  expect_output(
    print(s2geography("MULTIPOINT ((-64 45), (30 10))")),
    "<MULTIPOINT \\(\\(-64 45\\), \\(30 10\\)\\)>"
  )

  expect_output(
    print(s2geography("LINESTRING (-64 45, 0 0)")),
    "<LINESTRING \\(-64 45, 0 0\\)>"
  )
  expect_output(
    print(s2geography("LINESTRING EMPTY")),
    "<LINESTRING EMPTY>"
  )
  expect_output(
    print(s2geography("MULTILINESTRING ((-64 45, 0 0), (0 1, 2 3))")),
    "<MULTILINESTRING \\(\\(-64 45, 0 0), \\(0 1, 2 3\\)\\)>"
  )

  expect_output(print(s2geography("POLYGON EMPTY"), "<POLYGON EMPTY>"))
  expect_output(
    print(s2geography("POLYGON ((0 0, 10 0, 10 10, 0 10, 0 0))")),
    "<POLYGON \\(\\(0 0, 10 0, 10 10"
  )
  expect_output(
    print(s2geography("MULTIPOLYGON (((0 0, 10 0, 10 10, 0 10, 0 0)))")),
    "<POLYGON \\(\\(0 0, 10 0, 10 10"
  )
  expect_output(
    print(
      s2geography("MULTIPOLYGON (
        ((40 40, 20 45, 45 30, 40 40)),
        ((20 35, 10 30, 10 10, 30 5, 45 20, 20 35), (30 20, 20 15, 20 25, 30 20))
      )")
    ),
    "<MULTIPOLYGON"
  )

  expect_output(
    print(s2geography("GEOMETRYCOLLECTION (POINT (-64 45))")),
    "<GEOMETRYCOLLECTION \\(POINT \\(-64 45\\)\\)>"
  )

  expect_output(
    print(
      s2geography(
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

  expect_output(print(s2geography("GEOMETRYCOLLECTION EMPTY")), "<GEOMETRYCOLLECTION EMPTY>")
})

test_that("nested ring depths are correctly exported", {
  # polygon with hole
  expect_output(
    print(
      s2geography("MULTIPOLYGON (
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
      s2geography("MULTIPOLYGON (
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
  polygon_with_bad_hole_nested <- s2geography("MULTIPOLYGON (
    ((40 40, 20 45, 45 30, 40 40)),
    (
      (20 35, 10 30, 10 10, 30 5, 45 20, 20 35),
      (30 20, 20 25, 20 15, 30 20)
    )
  )", oriented = FALSE)

  expect_false(s2_intersects(polygon_with_bad_hole_nested, "POINT (23 19.5)"))

  expect_error(
    s2geography("MULTIPOLYGON (
      ((40 40, 20 45, 45 30, 40 40)),
      (
        (20 35, 10 30, 10 10, 30 5, 45 20, 20 35),
        (30 20, 20 25, 20 15, 30 20)
      )
    )", oriented = TRUE),
    "Inconsistent loop orientations"
  )
})
