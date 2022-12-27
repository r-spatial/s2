
test_that("s2_cell class works", {
  expect_s3_class(new_s2_cell(double()), "s2_cell")
  expect_s3_class(new_s2_cell(NA_real_), "s2_cell")
  expect_true(is.na(new_s2_cell(NA_real_)))
  expect_identical(as_s2_cell(s2_cell()), s2_cell())
  expect_identical(
    as.list(new_s2_cell(NA_real_)),
    list(new_s2_cell(NA_real_))
  )
})

test_that("invalid and sentinel values work as expected", {
  expect_false(s2_cell_is_valid(s2_cell_sentinel()))
  expect_false(s2_cell_is_valid(s2_cell_invalid()))
  expect_false(is.na(s2_cell_sentinel()))
  expect_false(is.na(s2_cell_invalid()))
  expect_true(s2_cell_sentinel() > s2_cell_invalid())
})

test_that("sort() and unique() work", {
  # temporary workaround for broken c() in wk
  expect_identical(
    unique(new_s2_cell(c(unclass(s2_cell_sentinel()), NA, 0, 0, unclass(s2_cell("5"))))),
    new_s2_cell(c(0, unclass(s2_cell("5")), NA, unclass(s2_cell_sentinel())))
  )

  expect_identical(
    sort(new_s2_cell(c(unclass(s2_cell_sentinel()), NA, 0, 0, unclass(s2_cell("5"))))),
    new_s2_cell(c(0, 0, unclass(s2_cell("5")), NA, unclass(s2_cell_sentinel())))
  )

  expect_identical(
    sort(
      new_s2_cell(c(unclass(s2_cell_sentinel()), NA, 0, 0, unclass(s2_cell("5")))),
      decreasing = TRUE
    ),
    rev(new_s2_cell(c(0, 0, unclass(s2_cell("5")), NA, unclass(s2_cell_sentinel()))))
  )
})

test_that("geography exporters work", {
  expect_identical(
    s2_as_text(s2_cell_center(as_s2_cell(s2_lnglat(c(-64, NA), c(45, NA)))), precision = 5),
    c("POINT (-64 45)", NA)
  )

  expect_identical(
    s2_as_text(s2_cell_polygon(as_s2_cell(s2_lnglat(c(-64, NA), c(45, NA)))), precision = 5),
    c("POLYGON ((-64 45, -64 45, -64 45, -64 45, -64 45))", NA)
  )

  expect_identical(
    s2_as_text(s2_cell_boundary(as_s2_cell(s2_lnglat(c(-64, NA), c(45, NA)))), precision = 5),
    c("LINESTRING (-64 45, -64 45, -64 45, -64 45, -64 45)", NA)
  )

  expect_equal(
    s2_x(s2_cell_vertex(s2_cell(c("5", "5", "5", NA)), k = c(0, 1, NA, 1))),
    c(45, 135, NA, NA)
  )
})

test_that("s2_cell_is_valid() works", {
  expect_identical(
    s2_cell_is_valid(new_s2_cell(double())),
    logical()
  )
  expect_identical(
    s2_cell_is_valid(new_s2_cell(NA_real_)),
    FALSE
  )
})

test_that("Ops, Math, and Summary errors for non-meaningful s2_cell() ops", {
  expect_error(s2_cell("X") + 1, "not meaningful")
  expect_error(abs(s2_cell("X")), "not meaningful")
  expect_error(all(s2_cell("X")), "not meaningful")
})

test_that("Ops, Math, and Summary ops work", {
  expect_identical(
    s2_cell(c("5", "5", "x", "x", "x", NA, NA)) ==
      s2_cell(c("5", "x", "5", "x", NA, "x", NA)),
    c(TRUE, FALSE, FALSE, TRUE, NA, NA, NA)
  )

  expect_identical(
    s2_cell(c("5", "5", "x", "x", "x", NA, NA)) !=
      s2_cell(c("5", "x", "5", "x", NA, "x", NA)),
    c(FALSE, TRUE, TRUE, FALSE, NA, NA, NA)
  )

  # 'invalid' is 0
  expect_identical(
    s2_cell(c("5", "5", "x", "x", "x", NA, NA)) >
      s2_cell(c("5", "x", "5", "x", NA, "x", NA)),
    c(FALSE, TRUE, FALSE, FALSE, NA, NA, NA)
  )

  expect_identical(
    s2_cell(c("5", "5", "x", "x", "x", NA, NA)) >=
      s2_cell(c("5", "x", "5", "x", NA, "x", NA)),
    c(TRUE, TRUE, FALSE, TRUE, NA, NA, NA)
  )

  expect_identical(
    s2_cell(c("5", "5", "x", "x", "x", NA, NA)) <
      s2_cell(c("5", "x", "5", "x", NA, "x", NA)),
    c(FALSE, FALSE, TRUE, FALSE, NA, NA, NA)
  )

  expect_identical(
    s2_cell(c("5", "5", "x", "x", "x", NA, NA)) <=
      s2_cell(c("5", "x", "5", "x", NA, "x", NA)),
    c(TRUE, FALSE, TRUE, TRUE, NA, NA, NA)
  )

  expect_identical(
    cummax(s2_cell(c("5", "x", "5", "x", NA, "x", NA))),
           s2_cell(c("5", "5", "5", "5", NA, NA, NA))
  )

  expect_identical(
    cummin(s2_cell(c("5", "x", "5", "x", NA, "x", NA))),
           s2_cell(c("5", "x", "x", "x", NA, NA, NA))
  )

  expect_identical(
    range(s2_cell(c("5", "x", "5", "x", NA, "x", NA)), na.rm = TRUE),
    s2_cell(c("X", "5"))
  )

  expect_identical(
    max(s2_cell(c("5", "x", "5", "x", NA, "x", NA)), na.rm = TRUE),
    s2_cell("5")
  )

  expect_identical(
    min(s2_cell(c("5", "x", "5", "x", NA, "x", NA)), na.rm = TRUE),
    s2_cell("X")
  )

  expect_identical(
    range(s2_cell(c("5", "x", "5", "x", NA, "x", NA))),
    s2_cell(c(NA_character_, NA_character_))
  )

  expect_identical(range(s2_cell()), s2_cell(c(NA_character_, NA_character_)))
  expect_identical(
    range(s2_cell(NA_character_), na.rm = TRUE),
    s2_cell(c(NA_character_, NA_character_))
  )
})

test_that("Binary ops are recycled at the C++ level", {
  expect_identical(s2_cell(rep("5", 2)) == s2_cell(rep("5", 2)), c(TRUE, TRUE))
  expect_identical(s2_cell(rep("5", 1)) == s2_cell(rep("5", 2)), c(TRUE, TRUE))
  expect_identical(s2_cell(rep("5", 2)) == s2_cell(rep("5", 1)), c(TRUE, TRUE))
  expect_error(s2_cell(rep("5", 2)) == s2_cell(rep("5", 0)), "Can't recycle")
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
  expect_error(as.data.frame(new_s2_cell(NA_real_)), "cannot coerce")
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
    c(TRUE, TRUE, FALSE, FALSE)
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

test_that("s2_cell() binary operators work", {
  cell <- s2_cell("4b5f6a7856889a33")
  expect_true(s2_cell_contains(s2_cell_parent(cell), cell))
  expect_false(s2_cell_contains(cell, s2_cell_parent(cell)))
  expect_identical(s2_cell_contains(s2_cell_sentinel(), s2_cell_sentinel()), NA)

  expect_equal(
    s2_cell_distance(
      as_s2_cell(s2_lnglat(0, 90)),
      as_s2_cell(s2_lnglat(0, 0)),
      radius = 1
    ),
    pi / 2
  )

  expect_equal(
    s2_cell_max_distance(
      as_s2_cell(s2_lnglat(0, 90)),
      as_s2_cell(s2_lnglat(0, 0)),
      radius = 1
    ),
    pi / 2
  )

  f1 <- s2_cell_parent(as_s2_cell(s2_lnglat(0, 0)), 0)
  f2 <- s2_cell_parent(as_s2_cell(s2_lnglat(0, 90)), 0)
  expect_equal(s2_cell_distance(f1, f2), 0)
  expect_equal(s2_cell_max_distance(f1, f2, radius = 1), pi)
  expect_identical(s2_cell_max_distance(s2_cell_sentinel(), s2_cell_sentinel()), NA_real_)
  expect_identical(s2_cell_distance(s2_cell_sentinel(), s2_cell_sentinel()), NA_real_)

  expect_false(
    s2_cell_may_intersect(
      as_s2_cell(s2_lnglat(0, 90)),
      as_s2_cell(s2_lnglat(0, 0))
    )
  )

  expect_true(s2_cell_may_intersect(s2_cell_parent(cell), cell))
  expect_true(s2_cell_may_intersect(cell, s2_cell_parent(cell)))
  expect_identical(s2_cell_may_intersect(s2_cell_sentinel(), s2_cell_sentinel()), NA)
})

test_that("s2_cell_common_ancestor_level() works", {
  cell <- s2_cell_parent(s2_cell("4b5f6a7856889a33"), 10)
  children <- s2_cell_child(cell, 0:4)
  expect_identical(
    s2_cell_common_ancestor_level(children, cell),
    c(rep(10L, 4), NA),
  )

  expect_identical(s2_cell_common_ancestor_level_agg(s2_cell()), NA_integer_)
  expect_identical(
    s2_cell_common_ancestor_level_agg(
      as_s2_cell(as_s2_geography(c("POINT (0 0)", "POINT (180 0)")))
    ),
    -1L
  )

  expect_identical(
    s2_cell_common_ancestor_level_agg(children, na.rm = TRUE),
    10L
  )

  expect_identical(
    s2_cell_common_ancestor_level_agg(children, na.rm = FALSE),
    NA_integer_
  )
})

