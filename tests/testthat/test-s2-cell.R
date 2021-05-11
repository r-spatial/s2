
test_that("s2_cell class works", {
  expect_s3_class(new_s2_cell(double()), "s2_cell")
  expect_s3_class(new_s2_cell(NA_real_), "s2_cell")
  expect_true(is.na(new_s2_cell(NA_real_)))
})

test_that("s2_cell_is_valid() works", {
  expect_identical(
    s2_cell_is_valid(new_s2_cell(double())),
    logical()
  )
  expect_identical(
    s2_cell_is_valid(new_s2_cell(NA_real_)),
    NA
  )
})

test_that("Ops, Math, and Summary errors for non-meaningful s2_cell() ops", {
  expect_error(s2_cell("X") + 1, "not meaningful")
  expect_error(abs(s2_cell("X")), "not meaningful")
  expect_error(all(s2_cell("X")), "not meaningful")
})

test_that("s2_cell is not numeric", {
  expect_false(is.numeric(s2_cell()))
})

test_that("s2_cell subsetting and concatenation work", {
  cells <- new_s2_cell(c(NA_real_, NA_real_))
  expect_identical(cells[1], new_s2_cell(NA_real_))
  expect_identical(cells[[1]], cells[1])
  expect_identical(
    c(cells, cells),
    new_s2_cell(c(NA_real_, NA_real_, NA_real_, NA_real_))
  )
  expect_identical(
    rep(cells, 2),
    new_s2_cell(c(NA_real_, NA_real_, NA_real_, NA_real_))
  )
  expect_identical(
    rep_len(cells, 4),
    new_s2_cell(c(NA_real_, NA_real_, NA_real_, NA_real_))
  )
  expect_error(
    c(new_s2_cell(c(NA_real_, NA_real_)), 1:5),
    "Can't combine"
  )
})

test_that("s2_cell() can be created from s2_geography()", {
  expect_identical(
    as_s2_cell(as_s2_geography("POINT (-64 45)")),
    s2_cell("4b59a0cd83b5de49")
  )
})

test_that("s2_cell() can be created from s2_lnglat()", {
  expect_identical(
    as_s2_cell(s2_lnglat(-64, 45)),
    s2_cell("4b59a0cd83b5de49")
  )
})

test_that("s2_cell() can be created from s2_point()", {
  expect_identical(
    as_s2_cell(as_s2_point(s2_lnglat(-64, 45))),
    s2_cell("4b59a0cd83b5de49")
  )
})

test_that("s2_cell() can be created from character", {
  expect_identical(
    as_s2_cell("4b59a0cd83b5de49"),
    as_s2_cell(s2_lnglat(-64, 45))
  )
})

test_that("subset-assignment works", {
  x <- as_s2_cell(c(NA, "4b59a0cd83b5de49", "4b5f6a7856889a33"))
  expect_identical(as.character(x[3]), "4b5f6a7856889a33")
  x[3] <- "4b59a0cd83b5de49"
  expect_identical(as.character(x[3]), "4b59a0cd83b5de49")

  x[[3]] <- "4b5f6a7856889a33"
  expect_identical(as.character(x[3]), "4b5f6a7856889a33")
})

test_that("s2_cell can be put into a data.frame", {
  expect_identical(
    data.frame(geom = new_s2_cell(NA_real_)),
    new_data_frame(list(geom = new_s2_cell(NA_real_)))
  )
  expect_error(as.data.frame(new_s2_cell(NA_real_)), "cannot coerce class")
})

test_that("s2_cell default format/print/str methods work", {
  expect_identical(
    format(s2_cell()),
    as.character(s2_cell())
  )
  expect_output(print(new_s2_cell(double())), "s2_cell")
  expect_output(print(new_s2_cell(NA_real_)), "s2_cell")

  expect_output(str(as_s2_cell(character())), "s2_cell\\[0\\]")
  expect_output(str(as_s2_cell(NA_character_)), "NA")
})

test_that("s2 cell exporters work", {
  expect_equal(
    as.data.frame(s2_cell_to_lnglat(s2_cell(c("4b59a0cd83b5de49", "x", NA)))),
    as.data.frame(c(s2_lnglat(-64, 45), s2_lnglat(NA, NA), s2_lnglat(NA, NA)))
  )

  expect_identical(
    s2_cell_debug_string(s2_cell(c("4b5f6a7856889a33", NA))),
    c("2/112233231103300223101010310121", NA)
  )
  expect_match(s2_cell_debug_string(s2_cell("X")), "Invalid")
})

test_that("s2_cell() accessors work", {
  expect_identical(
    s2_cell_is_valid(s2_cell(c("4b5f6a7856889a33", "5", "x", NA))),
    c(TRUE, TRUE, FALSE, NA)
  )

  expect_identical(
    s2_cell_level(s2_cell(c("4b5f6a7856889a33", "5", "x", NA))),
    c(30L, 0L, NA, NA)
  )

  expect_identical(
    s2_cell_is_leaf(s2_cell(c("4b5f6a7856889a33", "5", "x", NA))),
    c(TRUE, FALSE, NA, NA)
  )

  expect_identical(
    s2_cell_is_face(s2_cell(c("4b5f6a7856889a33", "5", "x", NA))),
    c(FALSE, TRUE, NA, NA)
  )

  expect_equal(
    s2_cell_area(s2_cell(c("5", "x", NA)), radius = 1),
    c(2 * pi / 3, NA, NA)
  )

  expect_equal(
    s2_cell_area_approx(s2_cell(c("5", "x", NA)), radius = 1),
    c(2 * pi / 3, NA, NA)
  )
})

test_that("s2_cell() transversers work", {
  expect_identical(
    s2_cell_parent(s2_cell(c("5", "4b5f6a7856889a33", "x", NA)), 0),
    s2_cell(c("5", "5", NA, NA))
  )

  leaf_manual <- function(x, children) {
    if (length(children) == 0) {
      x
    } else {
      leaf_manual(
        s2_cell_child(x, children[1]),
        children[-1]
      )
    }
  }

  # see output of s2_cell_debug_string()
  expect_identical(
    leaf_manual(
      s2_cell("5"),
      as.numeric(strsplit("112233231103300223101010310121", "")[[1]])
    ),
    s2_cell("4b5f6a7856889a33")
  )

  expect_identical(
    s2_cell_child(s2_cell(c("5", "5", "5", "x", NA)), c(-1, 4, NA, 0, 0)),
    s2_cell(as.character(c(NA, NA, NA, NA, NA)))
  )

  expect_identical(
    s2_cell_debug_string(s2_cell_edge_neighbour(s2_cell("5"), 0:3)),
    c("1/", "3/", "4/", "0/")
  )

  expect_identical(
    s2_cell_edge_neighbour(s2_cell(c("5", "5", "5", "x", NA)), c(-1, 4, NA, 0, 0)),
    s2_cell(as.character(c(NA, NA, NA, NA, NA)))
  )
})
