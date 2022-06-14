
test_that("s2_geography class works", {
  expect_s3_class(s2_geography(), "s2_geography")
  geog <- new_s2_geography(list(NULL))
  expect_output(print(geog), "s2_geography")
  expect_output(str(geog), "s2_geography")
  expect_identical(as_s2_geography(geog), geog)

  expect_identical(
    is.na(as_s2_geography(c("POINT (0 1)", NA_character_))),
    c(FALSE, TRUE)
  )

  # subset assignment
  geog2 <- geog
  geog2[1] <- geog
  expect_identical(geog2, geog)

  geog2 <- geog
  geog2[[1]] <- geog
  expect_identical(geog2, geog)
})

test_that("s2_geography vectors can be put in a data frame", {
  expect_identical(
    data.frame(geog = s2_geography()),
    new_data_frame(list(geog = s2_geography()))
  )
})

test_that("s2_geography vectors can't have other types of objects concatenated or asssigned", {
  geog <- new_s2_geography(list(NULL))
  expect_s3_class(c(geog, geog), "s2_geography")
  expect_error(c(geog, wk::wkt()), "Can't combine 'wk_vctr' objects")
  expect_error(geog[1] <- factor(1), "no applicable method")
  expect_error(geog[[1]] <- factor(1), "no applicable method")
})

test_that("s2_geography vectors can be created from s2_lnglat  and s2_point", {
  expect_wkt_equal(as_s2_geography(s2_lnglat(-64, 45)), "POINT (-64 45)")
  expect_wkt_equal(as_s2_geography(as_s2_point(s2_lnglat(-64, 45))), "POINT (-64 45)")
})

test_that("s2_geography vectors can be created from WKB and WKT", {
  wkb_point <- wk::as_wkb(wk::wkt("POINT (-64 45)", geodesic = TRUE))
  expect_output(print(as_s2_geography(wkb_point)), "POINT \\(-64 45\\)")
  expect_error(
    as_s2_geography(wk::as_wkb("LINESTRING (0 0, 1 1)")),
    "Cartesian wkb\\(\\)"
  )

  # empty, null, and point features are OK
  expect_identical(as_s2_geography(wk::wkb()), as_s2_geography(character()))
  expect_identical(as_s2_geography(wk::wkb(list(NULL))), as_s2_geography(NA_character_))
  expect_silent(as_s2_geography(wk::as_wkb("POINT (0 1)")))
  expect_silent(as_s2_geography(wk::as_wkb("MULTIPOINT (0 1)")))

  wkt_point <- wk::as_wkt(wk::wkt("POINT (-64 45)", geodesic = TRUE))
  expect_output(print(as_s2_geography(wkt_point)), "POINT \\(-64 45\\)")
  expect_error(
    as_s2_geography(wk::wkt("LINESTRING (0 0, 1 1)")),
    "Cartesian wkt\\(\\)"
  )

  # empty, null, and point features are OK
  expect_identical(as_s2_geography(wk::wkt()), as_s2_geography(character()))
  expect_identical(as_s2_geography(wk::wkt(NA_character_)), as_s2_geography(NA_character_))
  expect_silent(as_s2_geography(wk::wkt("POINT (0 1)")))
  expect_silent(as_s2_geography(wk::wkt("MULTIPOINT (0 1)")))

  # also test other classes commonly used to signify WKB or WKT
  expect_output(print(as_s2_geography(structure(wkb_point, class = "WKB")), "POINT \\(-64 45\\)"))
  expect_output(print(as_s2_geography(structure(wkb_point, class = "blob")), "POINT \\(-64 45\\)"))
})

test_that("s2_geography can be exported to WKB/WKT", {
  expect_wkt_equal(
    wk::as_wkb(as_s2_geography("POINT (-64 45)")),
    wk::as_wkb(wk::wkt("POINT (-64 45)", geodesic = TRUE)),
    precision = 10
  )
  expect_wkt_equal(
    wk::as_wkt(as_s2_geography("POINT (-64 45)")),
    wk::as_wkt(wk::wkt("POINT (-64 45)", geodesic = TRUE)),
    precision = 10
  )
})

test_that("s2_geography vectors can be created from wkt", {
  expect_output(print(as_s2_geography("POINT (-64 45)")), "POINT \\(-64 45\\)")
  expect_output(print(as_s2_geography("POINT EMPTY")), "POINT EMPTY")
  expect_output(
    print(as_s2_geography("MULTIPOINT ((-64 45), (30 10))")),
    "MULTIPOINT \\(\\(-64 45\\), \\(30 10\\)\\)"
  )

  expect_output(
    print(as_s2_geography("LINESTRING (-64 45, 0 0)")),
    "LINESTRING \\(-64 45, 0 0\\)"
  )
  expect_output(
    print(as_s2_geography("LINESTRING EMPTY")),
    "LINESTRING EMPTY"
  )
  expect_output(
    print(as_s2_geography("MULTILINESTRING ((-64 45, 0 0), (0 1, 2 3))")),
    "MULTILINESTRING \\(\\(-64 45, 0 0), \\(0 1, 2 3\\)\\)"
  )

  expect_output(print(as_s2_geography("POLYGON EMPTY"), "POLYGON EMPTY"))
  expect_output(
    print(as_s2_geography("POLYGON ((0 0, 10 0, 10 10, 0 10, 0 0))")),
    "POLYGON \\(\\(0 0, 10 0, 10 10"
  )
  expect_output(
    print(as_s2_geography("MULTIPOLYGON (((0 0, 10 0, 10 10, 0 10, 0 0)))")),
    "POLYGON \\(\\(0 0, 10 0, 10 10"
  )
  expect_output(
    print(
      as_s2_geography("MULTIPOLYGON (
        ((40 40, 20 45, 45 30, 40 40)),
        ((20 35, 10 30, 10 10, 30 5, 45 20, 20 35), (30 20, 20 15, 20 25, 30 20))
      )")
    ),
    "MULTIPOLYGON"
  )

  expect_output(
    print(as_s2_geography("GEOMETRYCOLLECTION (POINT (-64 45))")),
    "GEOMETRYCOLLECTION \\(POINT \\(-64 45\\)\\)"
  )

  expect_match(
    s2_as_text(
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
      )
    ),
    paste0(
      "GEOMETRYCOLLECTION.*?POINT.*?MULTIPOINT.*?LINESTRING.*?MULTILINESTRING.*?",
      "POLYGON.*?POLYGON.*?GEOMETRYCOLLECTION.*?POINT.*?MULTIPOINT"
    )
  )

  expect_output(print(as_s2_geography("GEOMETRYCOLLECTION EMPTY")), "GEOMETRYCOLLECTION EMPTY")
})

test_that("empty points are empty when imported from WKB", {
  wkb_empty <- wk::as_wkb("POINT EMPTY")
  expect_true(s2_is_empty(s2_geog_from_wkb(wkb_empty)))
})

test_that("nested ring depths are correctly exported", {
  # polygon with hole
  expect_match(
    s2_as_text(
      as_s2_geography("MULTIPOLYGON (
        ((40 40, 20 45, 45 30, 40 40)),
        (
          (20 35, 10 30, 10 10, 30 5, 45 20, 20 35),
          (30 20, 20 15, 20 25, 30 20)
        )
      )")
    ),
    "\\(20 35, 10 30, 10 10, 30 5, 45 20, 20 35\\), \\(30 20, 20 15, 20 25"
  )

  # polygon with a hole in a hole!
  expect_match(
    s2_as_text(
      as_s2_geography("MULTIPOLYGON (
        ((40 40, 20 45, 45 30, 40 40)),
        (
          (20 35, 10 30, 10 10, 30 5, 45 20, 20 35),
          (30 20, 20 15, 20 25, 30 20)
        ),
        ((27 21, 21 21, 21 16, 27 21))
      )")
    ),
    "\\(\\(27 21, 21 21, 21 16, 27 21\\)\\)\\)"
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

test_that("wk crs and geodesic methods are defined", {
  geog <- as_s2_geography("POINT (0 0)")

  expect_identical(wk::wk_crs(geog), wk::wk_crs_longlat())
  expect_true(wk::wk_is_geodesic(geog))

  expect_identical(wk::wk_set_crs(geog, wk::wk_crs_longlat()), geog)
  expect_identical(wk::wk_set_geodesic(geog, TRUE), geog)

  expect_warning(
    wk::wk_set_crs(geog, "EPSG:32620"),
    "is not supported"
  )

  expect_error(wk::wk_set_geodesic(geog, FALSE), "Can't set geodesic")
})
