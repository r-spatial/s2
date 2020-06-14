
test_that("s2_astext() works", {
  expect_identical(
    s2_astext("POINT (0.1234567890123456 0.1234567890123456)"),
    "POINT (0.1234567890123456 0.1234567890123456)"
  )
})

test_that("s2_asbinary() works", {
  expect_identical(
    s2_asbinary("POINT (0 0)", endian = 0),
    list(
      as.raw(
        c(0x00, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00,
          0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
          0x00, 0x00
        )
      )
    )
  )
})

test_that("s2_asbinary works on (multi)polygons", {
	geog <- s2data_countries()
	wkb <- s2_asbinary(geog)
	expect_identical(object.size(wkb), structure(183840, class = 'object_size'))
	expect_identical(length(wkb), length(geog))
})
