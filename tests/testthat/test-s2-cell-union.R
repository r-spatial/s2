
test_that("s2_cell_union() class works", {
  expect_s3_class(s2_cell_union(), "s2_cell_union")
  expect_s3_class(s2_cell_union(), "wk_vctr")

  x <- s2_cell_union()
  expect_identical(as_s2_cell_union(x), x)
  expect_output(expect_identical(str(x), x), "s2_cell_union")
  expect_output(expect_identical(print(x), x), "s2_cell_union")
  expect_identical(unlist(x), s2_cell())
})

test_that("as_s2_geography() for s2_cell_union works", {
  union <- s2_cell_union(c("4b59a0cd83b5de49", NA))
  geog <- as_s2_geography(union)
  expect_identical(
    s2_intersects(geog, s2_lnglat(c(-64, NA), c(45, NA))),
    c(TRUE, NA)
  )
  expect_identical(s2_dimension(geog), c(2L, NA))
})

test_that("s2_cell_union_normalize() works", {
  cell <- s2_cell_parent(as_s2_cell("4b59a0cd83b5de49"), 10)
  children <- s2_cell_union(list(s2_cell_child(cell, 0:3)))
  expect_identical(
    s2_cell_union_normalize(children),
    as_s2_cell_union(cell)
  )

  expect_identical(
    s2_cell_union_normalize(new_s2_cell_union(list(NULL))),
    new_s2_cell_union(list(NULL))
  )
})

test_that("s2_cell_union_contains() works", {
  cell_na <- s2_cell_union(s2_cell(NA))
  cell <- s2_cell_parent(as_s2_cell("4b59a0cd83b5de49"), 10)
  children <- as_s2_cell_union(s2_cell_child(cell, 0:3))

  expect_identical(
    s2_cell_union_contains(cell, c(children, cell_na)),
    c(rep(TRUE, 4), NA)
  )

  expect_identical(
    s2_cell_union_contains(cell_na, children),
    rep(NA, 4)
  )

  expect_identical(
    s2_cell_union_contains(c(children, cell_na), cell),
    c(rep(FALSE, 4), NA)
  )

  expect_identical(
    s2_cell_union_contains(children, cell_na),
    rep(NA, 4)
  )

  expect_error(
    s2_cell_union_contains(children, c(s2_cell_union(cell), cell_na)),
    "Can't recycle vectors"
  )
})

test_that("s2_cell_union_contains() works for cell y", {
  cell_na <- s2_cell(NA)
  cell <- s2_cell_parent(as_s2_cell("4b59a0cd83b5de49"), 10)
  children <- s2_cell_child(cell, 0:3)

  expect_identical(
    s2_cell_union_contains(cell, c(children, cell_na)),
    c(rep(TRUE, 4), NA)
  )

  expect_identical(
    s2_cell_union_contains(cell_na, children),
    rep(NA, 4)
  )

  expect_identical(
    s2_cell_union_contains(c(children, cell_na), cell),
    c(rep(FALSE, 4), NA)
  )

  expect_identical(
    s2_cell_union_contains(children, cell_na),
    rep(NA, 4)
  )

  expect_error(
    s2_cell_union_contains(children, c(children, cell_na)),
    "Incompatible lengths"
  )
})

test_that("s2_cell_union_intersects() works", {
  cell_na <- s2_cell_union(s2_cell(NA))
  cell <- s2_cell_parent(as_s2_cell("4b59a0cd83b5de49"), 10)
  children <- as_s2_cell_union(s2_cell_child(cell, 0:3))

  expect_identical(
    s2_cell_union_intersects(cell, c(children, cell_na)),
    c(rep(TRUE, 4), NA)
  )

  expect_identical(
    s2_cell_union_intersects(cell_na, children),
    rep(NA, 4)
  )

  expect_identical(
    s2_cell_union_intersects(c(children, cell_na), cell),
    c(rep(TRUE, 4), NA)
  )

  expect_identical(
    s2_cell_union_intersects(children, cell_na),
    rep(NA, 4)
  )

  expect_error(
    s2_cell_union_intersects(children, c(s2_cell_union(cell), cell_na)),
    "Can't recycle vectors"
  )
})

test_that("s2_cell_union_intersection|difference|union() works", {
  cell <- s2_cell_parent(as_s2_cell("4b59a0cd83b5de49"), 10)
  children <- as_s2_cell_union(s2_cell_child(cell, 0:3))

  expect_identical(
    s2_cell_union_intersection(cell, children[1]),
    children[1]
  )

  expect_identical(
    s2_cell_union_difference(cell, children[1]),
    s2_cell_union(list(unlist(children[2:4])))
  )

  expect_identical(
    s2_cell_union_union(
      s2_cell_union(list(unlist(children[1:2]))),
      s2_cell_union(list(unlist(children[3:4])))
    ),
    s2_cell_union(cell)
  )
})

test_that("s2_covering_cell_ids() works", {
  expect_length(unlist(s2_covering_cell_ids(s2_data_countries("France"))), 8)
  expect_length(
    unlist(s2_covering_cell_ids(s2_data_countries("France"), max_cells = 4)),
    4
  )
  expect_length(
    unlist(s2_covering_cell_ids(s2_data_countries("France"), interior = TRUE)),
    8
  )
  expect_identical(s2_covering_cell_ids(NA_character_), new_s2_cell_union(list(NULL)))
})
