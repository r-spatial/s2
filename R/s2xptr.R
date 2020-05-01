
#' Create S2 vectors of XPtr objects
#'
#' @param x A bare `list()` of external pointers
#' @param class A character vector subclass
#'
#' @return An object of class s2xptr
#' @export
#'
new_s2xptr <- function(x = list(), class = character()) {
  if (!is.list(x) || is.object(x)) {
    stop("x must be a bare list of XPtr objects")
  }

  class(x) <- union(class, "s2xptr")
  x
}

#' @rdname new_s2xptr
#' @export
validate_s2xptr <- function(x) {
  type <- vapply(x, typeof, character(1))
  valid_items <- type %in% c("externalptr", "NULL")
  if (any(!valid_items)) {
    stop(sprintf("Items must be externalptr objects or NULL"))
  }

  invisible(x)
}

#' @export
`[.s2xptr` <- function(x, i) {
  new_s2xptr(NextMethod(), class(x))
}

#' @export
`c.s2xptr` <- function(...) {
  xptr <- new_s2xptr(NextMethod(), class(..1))
  validate_s2xptr(xptr)
  xptr
}

#' @export
print.s2xptr <- function(x, ...) {
  cat(sprintf("<%s[%s]>\n", class(x)[1], length(x)))
  if (length(x) == 0) {
    return(invisible(x))
  }

  out <- stats::setNames(format(x), names(x))
  print(out, quote = FALSE)
  invisible(x)
}
