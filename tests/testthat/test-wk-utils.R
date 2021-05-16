
test_that("unprojection using a wk filter works", {
  expect_equal(
    wk::wk_handle(wk::xy(0, 0), s2_unprojection_filter(wk::xy_writer())),
    wk::xyz(1, 0, 0)
  )

  expect_identical(
    wk::wk_handle(
      wk::wkt("LINESTRING (0 0, 0 45, -60 45)"),
      s2_unprojection_filter(wk::wkt_format_handler(precision = 4))
    ),
    # 0.7071 ~ sqrt(2) / 2
    "LINESTRING Z (1 0 0, 0.7071 0 0.7071, 0.3536 -0.6124 0.7071)"
  )

  expect_identical(
    wk::wk_handle(
      wk::wkt("POLYGON ((0 0, 0 45, -60 45, 0 0))"),
      s2_unprojection_filter(wk::wkt_format_handler(precision = 4))
    ),
    # 0.7071 ~ sqrt(2) / 2
    "POLYGON Z ((1 0 0, 0.7071 0 0.7071, 0.3536 -0.6124 0.7071, 1 0 0))"
  )

  expect_identical(
    wk::wk_handle(wk::wkt(NA_character_), s2_unprojection_filter(wk::wkt_writer())),
    wk::wkt(NA_character_)
  )
})


