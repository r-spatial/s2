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

test_that("null external pointers do not crash in the handler", {
  skip_if_serialization_supported()

  geog <- as_s2_geography("POINT (0 1)")
  geog2 <- unserialize(serialize(geog, NULL))
  expect_error(
    wk::wk_void(geog2),
    "External pointer is not valid"
  )
})

test_that("ALTREP can be disabled", {
  skip_if_serialization_unsupported()
  on.exit(options(s2.disable_altrep = NULL))
  options(s2.disable_altrep = TRUE)

  geog <- as_s2_geography("POINT (0 1)")
  geog2 <- unserialize(serialize(geog, NULL))
  expect_error(
    wk::wk_void(geog2),
    "External pointer is not valid"
  )

  options(s2.disable_altrep = NULL)
})

test_that("s2_geog_point() can be correctly serialized", {
  skip_if_serialization_unsupported()

  expect_wkt_serializeable(s2_geog_point(
    -64, 45
  ))
})

test_that("s2_union() can be correctly serialized", {
  skip_if_serialization_unsupported()

  expect_wkt_serializeable(s2_union(
    "POINT (10 30)",
    "POINT (30 10)"
  ))
})
