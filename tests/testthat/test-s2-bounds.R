
test_that("s2_bounds_cap works", {
  cap_ant <- s2_data_countries("Antarctica")
  expect_s3_class(s2_bounds_cap(cap_ant), "data.frame")
  expect_identical(nrow(s2_bounds_cap(cap_ant)), 1L)
  expect_true(s2_bounds_cap(cap_ant)$lat < -80)
  expect_true(s2_bounds_cap(cap_ant)$angle > 20)

  expect_identical(nrow(s2_bounds_cap(s2_data_countries(c("Antarctica", "Netherlands")))), 2L)

  expect_true(s2_bounds_cap(s2_data_countries("Netherlands"))$angle < 2)
  expect_true(s2_bounds_cap(s2_data_countries("Fiji"))$angle < 2)
})

test_that("s2_bounds_rect works", {
  rect_ant <- s2_bounds_rect(s2_data_countries("Antarctica"))
  expect_s3_class(rect_ant, "data.frame")
  expect_named(rect_ant, c("lng_lo", "lat_lo", "lng_hi", "lat_hi"))
  expect_identical(nrow(s2_bounds_rect(s2_data_countries(c("Antarctica", "Netherlands")))), 2L)
  expect_equal(rect_ant$lng_lo, -180)
  expect_equal(rect_ant$lng_hi, 180)
  expect_equal(rect_ant$lat_lo, -90)
  expect_true(rect_ant$lat_hi < -60)

  expect_identical(nrow(s2_bounds_rect(s2_data_countries(c("Antarctica", "Netherlands")))), 2L)

  rect_nl <- s2_bounds_rect(s2_data_countries("Netherlands"))
  expect_true((rect_nl$lng_hi - rect_nl$lng_lo) < 4)

  rect_fiji <- s2_bounds_rect(s2_data_countries("Fiji"))
  expect_true(rect_fiji$lng_hi < rect_fiji$lng_lo)

  rect_multipoint <- s2_bounds_rect("MULTIPOINT(-179 0,179 1,-180 10)")
  expect_equal(rect_multipoint$lat_lo, 0)
  expect_equal(rect_multipoint$lat_hi, 10)
  expect_equal(rect_multipoint$lng_lo, 179)
  expect_equal(rect_multipoint$lng_hi, -179)

  rect_linestring <- s2_bounds_rect("LINESTRING(-179 0,179 1)")
  expect_equal(rect_linestring$lat_lo, 0)
  expect_equal(rect_linestring$lat_hi, 1)
  expect_equal(rect_linestring$lng_lo, 179)
  expect_equal(rect_linestring$lng_hi, -179)
})
