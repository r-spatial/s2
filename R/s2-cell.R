
#' Create S2 Cell vectors
#'
#' @param x The canonical S2 cell identifier as a character vector.
#' @param ... Passed to methods
#'
#' @return An object of class s2_cell
#' @export
#'
#' @examples
#' s2_cell("87754e64dffffff")
#'
s2_cell <- function(x = character()) {
  new_s2_cell(cpp_s2_cell_from_string(x))
}

#' @rdname s2_cell
#' @export
as_s2_cell <- function(x, ...) {
  UseMethod("as_s2_cell")
}

#' @rdname s2_cell
#' @export
as_s2_cell.character <- function(x, ...) {
  s2_cell(x)
}

#' @rdname s2_cell
#' @export
as_s2_cell.s2_geography <- function(x, ...) {
  cpp_s2_cell_from_lnglat(list(s2_x(x), s2_y(x)))
}

#' @rdname s2_cell
#' @export
as_s2_cell.s2_lnglat <- function(x, ...) {
  cpp_s2_cell_from_lnglat(as.data.frame(x))
}

#' @rdname s2_cell
#' @export
as_s2_cell.s2_point <- function(x, ...) {
  as_s2_cell(as_s2_lnglat(x))
}

# keep private for now
new_s2_cell <- function(x) {
  structure(x, class = c("s2_cell", "wk_vctr"))
}

#' @export
as.character.s2_cell <- function(x, ...) {
  cpp_s2_cell_to_string(x)
}

#' @export
format.s2_cell <- function(x, ...) {
  format(as.character(x), quote = FALSE, ...)
}

#' @export
`[<-.s2_cell` <- function(x, i, value) {
  replacement <- as_s2_cell(value)
  x <- unclass(x)
  x[i] <- replacement
  new_s2_cell(x)
}

#' @export
`[[<-.s2_cell` <- function(x, i, value) {
  x[i] <- value
  x
}

s2_cell_is_valid <- function(x) {
  cpp_s2_cell_is_valid(x)
}


