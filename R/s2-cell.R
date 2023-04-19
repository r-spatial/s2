
#' Create S2 Cell vectors
#'
#' The S2 cell indexing system forms the basis for spatial indexing
#' in the S2 library. On their own, S2 cells can represent points
#' or areas. As a union, a vector of S2 cells can approximate a
#' line or polygon. These functions allow direct access to the
#' S2 cell indexing system and are designed to have minimal overhead
#' such that looping and recursion have acceptable performance
#' when used within R code.
#'
#' Under the hood, S2 cell vectors are represented in R as vectors
#' of type [double()]. This works because S2 cell identifiers are
#' 64 bits wide, as are `double`s on all systems where R runs (The
#' same trick is used by the bit64 package to represent signed
#' 64-bit integers). As a happy accident, `NA_real_` is not a valid
#' or meaningful cell identifier, so missing value support in the
#' way R users might expect is preserved. It is worth noting that
#' the underlying value of `s2_cell_sentinel()` would normally be
#' considered `NA`; however, as it is meaningful and useful when
#' programming with S2 cells, custom `is.na()` and comparison methods
#' are implemented such that `s2_cell_sentinel()` is greater than
#' all valid S2 cells and not considered missing. Users can and should
#' implement compiled code that uses the underlying bytes of the
#' vector, ensuring that the class of any returned object that should
#' be interpreted in this way is constructed with `new_s2_cell()`.
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
s2_cell_sentinel <- function() {
  cpp_s2_cell_sentinel()
}

#' @rdname s2_cell
#' @export
s2_cell_invalid <- function() {
  new_s2_cell(0)
}

#' @rdname s2_cell
#' @export
s2_cell_sentinel <- function() {
  cpp_s2_cell_sentinel()
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
as_s2_cell.wk_xy <- function(x, ...) {
  cpp_s2_cell_from_lnglat(as_s2_lnglat(x))
}

#' @rdname s2_cell
#' @export
as_s2_cell.integer64 <- function(x, ...) {
  storage <- unclass(x)
  storage[is.na(x)] <- NA_real_
  new_s2_cell(storage)
}

#' @rdname s2_cell
#' @export
new_s2_cell <- function(x) {
  stopifnot(is.double(x))
  structure(x, class = c("s2_cell", "wk_vctr"))
}

# registered in zzz.R
as.integer64.s2_cell <- function(x, ...) {
  # We store 64-bit integegers the same way bit64 does so we can just set the
  # class attribute and propagate NA values in the way that bit64 expects them.
  x_is_na <- is.na(x)
  class(x) <- "integer64"
  x[x_is_na] <- bit64::NA_integer64_
  x
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
as.list.s2_cell <- function(x, ...) {
  lapply(NextMethod(), new_s2_cell)
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

#' @export
unique.s2_cell <- function(x, ...) {
  cpp_s2_cell_unique(x)
}

#' @export
sort.s2_cell <- function(x, decreasing = FALSE, ...) {
  cpp_s2_cell_sort(x, decreasing)
}

#' @export
is.na.s2_cell <- function(x) {
  cpp_s2_cell_is_na(x)
}

#' @export
is.numeric.s2_cell <- function(x, ...) {
  FALSE
}

#' @export
Ops.s2_cell <- function(e1, e2) {
  switch(
    .Generic,
    "==" = cpp_s2_cell_eq(e1, e2),
    "!=" = cpp_s2_cell_neq(e1, e2),
    "<" = cpp_s2_cell_lt(e1, e2),
    "<=" = cpp_s2_cell_lte(e1, e2),
    ">=" = cpp_s2_cell_gte(e1, e2),
    ">" = cpp_s2_cell_gt(e1, e2),
    stop("Arithmetic operations are not meaningful for type 's2_cell'", call. = FALSE)
  )
}

#' @export
Math.s2_cell <- function(x, ...) {
  switch(
    .Generic,
    "cummax" = cpp_s2_cell_cummax(x),
    "cummin" = cpp_s2_cell_cummin(x),
    stop("Arithmetic operations are not meaningful for type 's2_cell'", call. = FALSE)
  )
}

#' @export
Summary.s2_cell <- function(x, ..., na.rm = FALSE) {
  switch(
    .Generic,
    "min" = cpp_s2_cell_range(x, na.rm)[1],
    "max" = cpp_s2_cell_range(x, na.rm)[2],
    "range" = cpp_s2_cell_range(x, na.rm),
    stop("Arithmetic operations are not meaningful for type 's2_cell'", call. = FALSE)
  )
}

#' S2 cell operators
#'
#' @param x,y An [s2_cell()] vector
#' @param level An integer between 0 and 30, inclusive.
#' @param k An integer between 0 and 3
#' @param radius The radius to use (e.g., [s2_earth_radius_meters()])
#' @param na.rm Remove NAs prior to computing aggregate?
#' @export
#'
s2_cell_is_valid <- function(x) {
  cpp_s2_cell_is_valid(x)
}

# exporters

#' @rdname s2_cell_is_valid
#' @export
s2_cell_debug_string <- function(x) {
  cpp_s2_cell_debug_string(x)
}

#' @rdname s2_cell_is_valid
#' @export
s2_cell_to_lnglat <- function(x) {
  lnglat <- cpp_s2_cell_to_lnglat(x)
  s2_lnglat(lnglat[[1]], lnglat[[2]])
}

#' @rdname s2_cell_is_valid
#' @export
s2_cell_center <- function(x) {
  cpp_s2_cell_center(x)
}

#' @rdname s2_cell_is_valid
#' @export
s2_cell_boundary <- function(x) {
  s2_boundary(cpp_s2_cell_polygon(x))
}

#' @rdname s2_cell_is_valid
#' @export
s2_cell_polygon <- function(x) {
  cpp_s2_cell_polygon(x)
}

#' @rdname s2_cell_is_valid
#' @export
s2_cell_vertex <- function(x, k) {
  recycled <- recycle_common(x, k)
  cpp_s2_cell_vertex(recycled[[1]], recycled[[2]])
}

# accessors

#' @rdname s2_cell_is_valid
#' @export
s2_cell_level <- function(x) {
  cpp_s2_cell_level(x)
}

#' @rdname s2_cell_is_valid
#' @export
s2_cell_is_leaf <- function(x) {
  s2_cell_level(x) == 30L
}

#' @rdname s2_cell_is_valid
#' @export
s2_cell_is_face <- function(x) {
  s2_cell_level(x) == 0L
}

#' @rdname s2_cell_is_valid
#' @export
s2_cell_area <- function(x, radius = s2_earth_radius_meters()) {
  cpp_s2_cell_area(x) * radius ^ 2
}

#' @rdname s2_cell_is_valid
#' @export
s2_cell_area_approx <- function(x, radius = s2_earth_radius_meters()) {
  cpp_s2_cell_area_approx(x) * radius ^ 2
}

# transversers

#' @rdname s2_cell_is_valid
#' @export
s2_cell_parent <- function(x, level = -1L) {
  recycled <- recycle_common(x, level)
  cpp_s2_cell_parent(recycled[[1]], recycled[[2]])
}

#' @rdname s2_cell_is_valid
#' @export
s2_cell_child <- function(x, k) {
  recycled <- recycle_common(x, k)
  cpp_s2_cell_child(recycled[[1]], recycled[[2]])
}

#' @rdname s2_cell_is_valid
#' @export
s2_cell_edge_neighbour <- function(x, k) {
  recycled <- recycle_common(x, k)
  cpp_s2_cell_edge_neighbour(recycled[[1]], recycled[[2]])
}

# binary operators

#' @rdname s2_cell_is_valid
#' @export
s2_cell_contains <- function(x, y) {
  cpp_s2_cell_contains(x, y)
}

#' @rdname s2_cell_is_valid
#' @export
s2_cell_distance <- function(x, y, radius = s2_earth_radius_meters()) {
  recycled <- recycle_common(x, y, radius)
  cpp_s2_cell_distance(recycled[[1]], recycled[[2]]) * radius
}

#' @rdname s2_cell_is_valid
#' @export
s2_cell_max_distance <- function(x, y, radius = s2_earth_radius_meters()) {
  recycled <- recycle_common(x, y, radius)
  cpp_s2_cell_max_distance(recycled[[1]], recycled[[2]]) * radius
}

#' @rdname s2_cell_is_valid
#' @export
s2_cell_may_intersect <- function(x, y) {
  cpp_s2_cell_may_intersect(x, y)
}

#' @rdname s2_cell_is_valid
#' @export
s2_cell_common_ancestor_level <- function(x, y) {
  cpp_s2_cell_common_ancestor_level(x, y)
}

#' @rdname s2_cell_is_valid
#' @export
s2_cell_common_ancestor_level_agg <- function(x, na.rm = FALSE) {
  x_na <- is.na(x)
  if (any(x_na) && !na.rm) {
    return(NA_integer_)
  }

  cpp_s2_cell_common_ancestor_level_agg(x[!x_na])
}
