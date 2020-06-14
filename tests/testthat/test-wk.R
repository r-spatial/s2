
test_that("s2latlng can be exported to wkb", {
  wkb_point <- wk::as_wkb("POINT (-64 45)")
  expect_identical(as_wkb(s2latlng(45, -64), endian = 1), wkb_point)
  expect_identical(as_wkb(s2latlng(double(), double())[NA]), wk::wkb(list(NULL)))
})


