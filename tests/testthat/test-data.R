
test_that("s2data_country() works", {
  expect_is(s2data_countries("Germany"), "s2geography")
  expect_length(s2data_countries("Germany"), 1)
  expect_is(s2data_countries(), "s2geography")
  expect_length(s2data_countries(), 177)
})
