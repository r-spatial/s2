
#' Create vectors of XPtr objects
#'
#' @param x A bare `list()` of external pointers
#' @param class A character vector subclass
#' @param ... Unused
#'
#' @return An object of class s2_xptr
#' @noRd
#'
new_s2_xptr <- function(x = list(), class = character()) {
  if (!is.list(x) || is.object(x)) {
    stop("x must be a bare list of 'externalptr' objects")
  }

  class(x) <- union(class, "s2_xptr")
  x
}

validate_s2_xptr <- function(x) {
  type <- vapply(unclass(x), typeof, character(1))
  valid_items <- type %in% c("externalptr", "NULL")
  if (any(!valid_items)) {
    stop("Items must be externalptr objects or NULL")
  }

  invisible(x)
}

#' @export
`[.s2_xptr` <- function(x, i) {
  new_s2_xptr(NextMethod(), class(x))
}

# makes lapply() along these vectors possible
#' @export
`[[.s2_xptr` <- function(x, i) {
  x[i]
}

#' @export
`c.s2_xptr` <- function(...) {
  # make sure all items inherit the same top-level class
  dots <- list(...)
  inherits_first <- vapply(dots, inherits, class(dots[[1]])[1], FUN.VALUE = logical(1))
  if (!all(inherits_first)) {
    stop(sprintf("All items must inherit from '%s'", class(dots[[1]])[1]))
  }

  xptr <- new_s2_xptr(NextMethod(), class(dots[[1]]))
  validate_s2_xptr(xptr)
  xptr
}

#' @export
rep.s2_xptr <- function(x, ...) {
  if (length(x) == 0) {
    new_s2_xptr(list(), class(x))
  } else {
    new_s2_xptr(NextMethod(), class(x))
  }
}

#' @method rep_len s2_xptr
#' @export
rep_len.s2_xptr <- function(x, length.out) {
  rep(x, length.out = length.out)
}

#' @export
print.s2_xptr <- function(x, ...) {
  cat(sprintf("<%s[%s]>\n", class(x)[1], length(x)))
  if (length(x) == 0) {
    return(invisible(x))
  }

  out <- stats::setNames(format(x, ...), names(x))
  print(out, quote = FALSE)
  invisible(x)
}
