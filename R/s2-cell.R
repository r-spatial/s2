
#' Create S2 Cell vectors
#'
#' @param x The canonical S2 cell identifier as a character vector.
#' @param ... Passed to methods
#'
#' @return An object of class s2_cell
#' @export
#'
#' @examples
#' s2_cell("4b59a0cd83b5de49")
#' as_s2_cell(s2_lnglat(-64, 45))
#' as_s2_cell(s2_data_cities("Ottawa"))
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
as_s2_cell.s2_cell <- function(x, ...) {
  x
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

#' S2 cell operators
#'
#' @param x,y An [s2_cell()] vector
#' @param level An integer between 0 and 30, inclusive.
#' @param k An integer between 1 and 4
#' @export
#'
s2_cell_is_valid <- function(x) {
  cpp_s2_cell_is_valid(as_s2_cell(x))
}

# exporters

#' @rdname s2_cell_is_valid
#' @export
s2_cell_to_lnglat <- function(x) {
  lnglat <- cpp_s2_cell_to_lnglat(as_s2_cell(x))
  s2_lnglat(lnglat[[1]], lnglat[[2]])
}

#' @rdname s2_cell_is_valid
#' @export
s2_cell_center <- function(x) {

}

#' @rdname s2_cell_is_valid
#' @export
s2_cell_boundary <- function(x) {

}

#' @rdname s2_cell_is_valid
#' @export
s2_cell_polygon <- function(x) {

}

#' @rdname s2_cell_is_valid
#' @export
s2_cell_vertex <- function(x, k) {

}

#' @rdname s2_cell_is_valid
#' @export
s2_cell_edge <- function(x, k) {

}

#' @rdname s2_cell_is_valid
#' @export
s2_cell_vertices <- function(x) {

}

# accessors

#' @rdname s2_cell_is_valid
#' @export
s2_cell_level <- function(x) {

}

#' @rdname s2_cell_is_valid
#' @export
s2_cell_is_leaf <- function(x) {

}

#' @rdname s2_cell_is_valid
#' @export
s2_cell_area <- function(x) {

}

#' @rdname s2_cell_is_valid
#' @export
s2_cell_area_approx <- function(x) {

}

# binary operators

#' @rdname s2_cell_is_valid
#' @export
s2_cell_contains <- function(x, y) {

}

#' @rdname s2_cell_is_valid
#' @export
s2_cell_distance <- function(x, y) {

}

#' @rdname s2_cell_is_valid
#' @export
s2_cell_boundary_distance <- function(x, y) {

}

#' @rdname s2_cell_is_valid
#' @export
s2_cell_max_distace <- function(x, y) {

}

#' @rdname s2_cell_is_valid
#' @export
s2_cell_may_intersect <- function(x, y) {

}

# transversers

#' @rdname s2_cell_is_valid
#' @export
s2_cell_parent <- function(x, level) {

}

#' @rdname s2_cell_is_valid
#' @export
s2_cell_child <- function(x, k) {

}

#' @rdname s2_cell_is_valid
#' @export
s2_cell_edge_neighbour <- function(x, k) {

}

#' @rdname s2_cell_is_valid
#' @export
s2_cell_vertex_neighbour <- function(x, k) {

}

#' @rdname s2_cell_is_valid
#' @export
s2_cell_neighbour <- function(x, k) {

}
