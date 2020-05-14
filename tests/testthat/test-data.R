
test_that("s2data_country() works", {
  expect_is(s2data_country("Germany"), "s2polygon")
  expect_length(s2data_country("Germany"), 3)
  expect_is(s2data_country(), "s2polygon")
  expect_length(s2data_country(), 1135)
})
