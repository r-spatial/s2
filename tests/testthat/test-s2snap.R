test_that("setting & getting snap level works", {
  sn = s2_get_snaplevel()
  expect_equal(sn, -1L)
  s2_set_snaplevel(25)
  expect_error(s2_set_snaplevel(31))
  expect_error(s2_set_snaplevel(-2))
  s2_set_snaplevel(sn)
})

