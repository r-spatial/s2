
test_that("s2_options() works", {
  expect_is(s2_options(), "s2_options")
  expect_named(s2_options(), c("polygon_model", "polyline_model", "snap", "snap_radius"))
})

test_that("s2_options() errors are readable", {
  expect_error(s2_intersects("POINT EMPTY", "POINT EMPTY", options = NULL), "must be created using")

  expect_error(
    s2_intersects("POINT EMPTY", "POINT EMPTY", options = s2_options(polygon_model = 5)),
    "Invalid value for polygon model"
  )
  expect_error(
    s2_intersects("POINT EMPTY", "POINT EMPTY", options = s2_options(polyline_model = 5)),
    "Invalid value for polyline model"
  )

  expect_error(
    s2_intersects("POINT EMPTY", "POINT EMPTY", options = s2_options(polygon_model = NULL)),
    "Error setting.*?polygon_model"
  )
  expect_error(
    s2_intersects("POINT EMPTY", "POINT EMPTY", options = s2_options(polyline_model = NULL)),
    "Error setting.*?polyline_model"
  )
  expect_error(
    s2_intersects("POINT EMPTY", "POINT EMPTY", options = s2_options(snap = 5)),
    "must be specified using s2_snap"
  )
  expect_error(
    s2_intersects("POINT EMPTY", "POINT EMPTY", options = s2_options(snap_radius = NULL)),
    "Error setting.*?snap_radius"
  )
})
