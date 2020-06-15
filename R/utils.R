
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

expect_wkt_equal <- function(x, y, precision = 16) {
  testthat::expect_equal(
    s2_geography_to_wkt(as_s2_geography(x), precision = precision, trim = TRUE),
    s2_geography_to_wkt(as_s2_geography(y), precision = precision, trim = TRUE)
  )
}

expect_near <- function(x, y, epsilon) {
  testthat::expect_true(abs(y - x) < epsilon)
}
