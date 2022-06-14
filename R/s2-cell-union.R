
#' Create S2 Cell Union vectors
#'
#' @param x A `list()` of [s2_cell()] vectors.
#' @param ... Passed to S3 methods
#'
#' @return An object of class "s2_cell_union".
#' @export
#'
s2_cell_union <- function(x = list()) {
  x <- as.list(x)
  input_na <- vapply(x, is.null, logical(1))
  union <- vector("list", length(x))
  union[input_na] <- list(NULL)
  union[!input_na] <- lapply(x[!input_na], as_s2_cell)
  new_s2_cell_union(union)
}

#' @rdname s2_cell_union
#' @export
as_s2_geography.s2_cell_union <- function(x, ...) {
  new_s2_geography(cpp_s2_geography_from_cell_union(as_s2_cell_union(x)))
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
  cpp_s2_cell_to_cell_union(x)
}

#' @rdname s2_cell_union
#' @export
as_s2_cell_union.character <- function(x, ...) {
  split <- strsplit(x, "\\s*;\\s*")
  split[is.na(x)] <- list(NULL)
  s2_cell_union(split)
}

new_s2_cell_union <- function(x) {
  stopifnot(typeof(x) == "list")
  structure(x, class = c("s2_cell_union", "wk_vctr"))
}

#' @export
is.na.s2_cell_union <- function(x, ...) {
  cpp_s2_cell_union_is_na(x)
}

#' @export
format.s2_cell_union <- function(x, ...) {
  formatted <- vapply(
    unclass(x),
    function(e) paste0(as.character(e), collapse = ";"),
    character(1)
  )

  formatted[is.na(x)] <- "<NA>"
  formatted
}

#' @export
as.character.s2_cell_union <- function(x, ...) {
  formatted <- vapply(
    unclass(x),
    function(e) paste0(as.character(e), collapse = ";"),
    character(1)
  )

  formatted[is.na(x)] <- NA_character_
  formatted
}

#' @export
print.s2_cell_union <- function(x, ...) {
  utils::str(x, ...)
  invisible(x)
}

#' @method unlist s2_cell_union
#' @export
unlist.s2_cell_union <- function(x, recursive = TRUE, use.names = TRUE) {
  unlisted <- unlist(unclass(x), recursive = recursive, use.names = use.names)
  new_s2_cell(as.double(unlisted))
}


#' @importFrom utils str
#' @export
str.s2_cell_union <- function(object, ..., indent.str = "") {
  cat(sprintf("%s<s2_cell_union[%d]>\n%s", indent.str, length(object), indent.str))
  str(unclass(object), ..., indent.str = indent.str)
  invisible(object)
}



#' S2 cell union operators
#'
#' @param x,y An [s2_geography][as_s2_geography] or [s2_cell_union()].
#' @param min_level,max_level The minimum and maximum levels to constrain the
#'   covering.
#' @param max_cells The maximum number of cells in the covering. Defaults to
#'   8.
#' @param buffer A distance to buffer outside the geography
#' @param interior Use `TRUE` to force the covering inside the geography.
#' @inheritParams s2_cell_is_valid
#'
#' @export
#'
s2_cell_union_normalize <- function(x) {
  cpp_s2_cell_union_normalize(as_s2_cell_union(x))
}

#' @rdname s2_cell_union_normalize
#' @export
s2_cell_union_contains <- function(x, y) {
  if (inherits(y, "s2_cell")) {
    recycled <- recycle_common(as_s2_cell_union(x), y)
    cpp_s2_cell_union_contains_cell(recycled[[1]], recycled[[2]])
  } else {
    cpp_s2_cell_union_contains(as_s2_cell_union(x), as_s2_cell_union(y))
  }
}

#' @rdname s2_cell_union_normalize
#' @export
s2_cell_union_intersects <- function(x, y) {
  cpp_s2_cell_union_intersects(as_s2_cell_union(x), as_s2_cell_union(y))
}

#' @rdname s2_cell_union_normalize
#' @export
s2_cell_union_intersection <- function(x, y) {
  cpp_s2_cell_union_intersection(as_s2_cell_union(x), as_s2_cell_union(y))
}

#' @rdname s2_cell_union_normalize
#' @export
s2_cell_union_union <- function(x, y) {
  cpp_s2_cell_union_union(as_s2_cell_union(x), as_s2_cell_union(y))
}

#' @rdname s2_cell_union_normalize
#' @export
s2_cell_union_difference <- function(x, y) {
  cpp_s2_cell_union_difference(as_s2_cell_union(x), as_s2_cell_union(y))
}

#' @rdname s2_cell_union_normalize
#' @export
s2_covering_cell_ids <- function(x, min_level = 0, max_level = 30,
                                 max_cells = 8, buffer = 0,
                                 interior = FALSE,
                                 radius = s2_earth_radius_meters()) {
  recycled <- recycle_common(as_s2_geography(x), buffer / radius)
  cpp_s2_covering_cell_ids(
    recycled[[1]],
    min_level,
    max_level,
    max_cells,
    recycled[[2]],
    interior
  )
}

#' @rdname s2_cell_union_normalize
#' @export
s2_covering_cell_ids_agg <- function(x, min_level = 0, max_level = 30,
                                     max_cells = 8, buffer = 0,
                                     interior = FALSE,
                                     radius = s2_earth_radius_meters(),
                                     na.rm = FALSE) {
  distance <- as.numeric(buffer / radius)
  stopifnot(length(distance) == 1)
  if (is.na(distance)) {
    return(new_s2_cell_union(list(NULL)))
  }

  cpp_s2_covering_cell_ids_agg(
    as_s2_geography(x),
    min_level,
    max_level,
    max_cells,
    distance,
    interior,
    na.rm
  )
}
