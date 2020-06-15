
test_that("s2_as_text() works", {
  expect_identical(
    s2_as_text("POINT (0.1234567890123456 0.1234567890123456)"),
    "POINT (0.1234567890123456 0.1234567890123456)"
  )
})

test_that("s2_as_binary() works", {
  expect_identical(
    s2_as_binary("POINT (0 0)", endian = 0),
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

test_that("s2_as_binary works on (multi)polygons", {
	geog <- s2_data_countries()
	wkb <- s2_as_binary(geog)

	expect_identical(
	  sum(vapply(wkb, length, integer(1))),
	  173318L
	)
	expect_identical(length(wkb), length(geog))
})
