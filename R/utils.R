
recycle_common <- function(...) {
  dots <- list(...)
  lengths <- vapply(dots, length, integer(1))
  non_constant_lengths <- unique(lengths[lengths != 1])
  if (length(non_constant_lengths) == 0) {
    final_length <- 1
  } else if(length(non_constant_lengths) == 1) {
    final_length <- non_constant_lengths
  } else {
    lengths_label <- paste0(non_constant_lengths, collapse = ", ")
    stop(sprintf("Incompatible lengths: %s", lengths_label))
  }

  lapply(dots, rep_len, final_length)
}

# The problems object is generated when building an s2_geography():
# instead of attaching to the object as an attribute, this function is
# called from Rcpp if there were any problems to format them in a
# human-readable way. This is a little crude and is a shortcut
# in anticipation of a rewrite of the constructors based on the new
# wk C API.
stop_problems <- function(feature_id, problem) {
  n <- length(feature_id)
  feature_label <- if (n != 1) "features" else "feature"

  if (n > 10) {
    feature_id <- feature_id[1:10]
    problem <- problem[1:10]
    more <- sprintf("\n...and %s more", n - 10)
  } else {
    more <- ""
  }

  msg <- sprintf(
    "Found %d %s with invalid spherical geometry.\n%s%s",
    n, feature_label,
    paste0("[", feature_id + 1, "] ", problem , collapse = "\n"),
    more
  )

  stop(msg, call. = FALSE)
}

expect_wkt_equal <- function(x, y, precision = 16) {
  testthat::expect_equal(
    s2_geography_to_wkt(as_s2_geography(x), precision = precision, trim = TRUE),
    s2_geography_to_wkt(as_s2_geography(y), precision = precision, trim = TRUE)
  )
}

expect_near <- function(x, y, epsilon) {
  testthat::expect_true(abs(y - x) < epsilon)
}
