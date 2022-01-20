
#' Create S2 Cell Union vectors
#'
#' @param x A `list()` of [s2_cell()] vectors.
#' @param ... Passed to S3 methods
#'
#' @return An object of class "s2_cell_union".
#' @export
#'
s2_cell_union <- function(x = list()) {
  new_s2_cell_union(lapply(x, as_s2_cell))
}

#' @rdname s2_cell_union
#' @export
as_s2_cell_union <- function(x, ...) {
  UseMethod("as_s2_cell_union")
}

#' @rdname s2_cell_union
#' @export
as_s2_cell_union.s2_cell_union <- function(x, ...) {
  x
}

#' @rdname s2_cell_union
#' @export
as_s2_cell_union.s2_cell <- function(x, ...) {
  new_s2_cell_union(list(x))
}

new_s2_cell_union <- function(x) {
  stopifnot(typeof(x) == "list")
  structure(x, class = c("s2_cell_union", "wk_vctr"))
}

#' @export
print.s2_cell_union <- function(x, ...) {
  str(x, ...)
  invisible(x)
}


#' @export
str.s2_cell_union <- function(x, ..., indent.str = "") {
  cat(sprintf("%s<s2_cell_union[%d]>\n%s", indent.str, length(x), indent.str))
  str(unclass(x), ..., indent.str = indent.str)
  invisible(x)
}

#' @rdname s2_cell_union
#' @export
s2_cell_union_normalize <- function(x) {
  cpp_s2_cell_union_normalize(as_s2_cell_union(x))
}


