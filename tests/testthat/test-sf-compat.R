
test_that("sf objects can be imported to s2", {
  skip_if_not_installed("sf")

  point <- sf_st_sfc(sf_st_point(c(-64, 45)), crs = 4326)
  line <- sf_st_sfc(sf_st_linestring(rbind(c(-64, 45), c(8, 52))), crs = 4326)
  polygon <- sf_st_sfc(
    sf_st_polygon(
      list(
        cbind(c(20, 35, 30, 20), c(30, 35, 20, 30)),
        cbind(c(35, 45, 15, 10, 35), c(10, 45, 40, 20, 10))
      )
    ),
    crs = 4326
  )

  point_roundtrip <- sf_st_as_sfc(s2latlng(point))
  line_roundtrip <- sf_st_as_sfc(s2polyline(line))
  polygon_roudtrip <- sf_st_as_sfc(s2polygon(polygon))

  expect_identical(class(point), class(point_roundtrip))
  expect_identical(class(line), class(line_roundtrip))
  expect_identical(class(polygon), class(polygon_roudtrip))

  #expect_identical(sf_st_crs(point)$espg, sf_st_crs(point_roundtrip)$espg)
  #expect_identical(sf_st_crs(line)$espg, sf_st_crs(line_roundtrip)$espg)
  #expect_identical(sf_st_crs(polygon)$espg, sf_st_crs(polygon_roudtrip)$espg)

  expect_equal(sf_st_coordinates(point), sf_st_coordinates(point_roundtrip))
  expect_equal(sf_st_coordinates(line), sf_st_coordinates(line_roundtrip))
  # rings may be reordered & reversed, so the following does not have to hold:
  # expect_equal(sf_st_coordinates(polygon), sf_st_coordinates(polygon_roudtrip))
})
