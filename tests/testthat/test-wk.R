
test_that("s2_latlng can be exported to wkb", {
  wkb_point <- wk::as_wkb("POINT (-64 45)")
  expect_identical(as_wkb(as_s2_latlng(45, -64), endian = 1), wkb_point)
  expect_identical(as_wkb(as_s2_latlng(double(), double())[NA]), wk::wkb(list(NULL)))
})


