
test_that("s2_cell_union() class works", {
  expect_s3_class(s2_cell_union(), "s2_cell_union")
  expect_s3_class(s2_cell_union(), "wk_vctr")

  x <- s2_cell_union()
  expect_identical(as_s2_cell_union(x), x)
  expect_output(expect_identical(str(x), x), "s2_cell_union")
  expect_output(expect_identical(print(x), x), "s2_cell_union")
})

test_that("s2_cell_union_normalize() works", {
  cell <- s2_cell_parent(as_s2_cell("4b59a0cd83b5de49"), 10)
  children <- s2_cell_child(cell, 0:3)
  expect_identical(
    s2_cell_union_normalize(children),
    as_s2_cell_union(cell)
  )

  expect_identical(
    s2_cell_union_normalize(new_s2_cell_union(list(NULL))),
    new_s2_cell_union(list(NULL))
  )
})
