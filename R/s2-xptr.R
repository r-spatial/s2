
#' Create vectors of XPtr objects
#'
#' @param x A bare `list()` of external pointers
#' @param class A character vector subclass
#' @param ... Unused
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
  type <- vapply(unclass(x), typeof, character(1))
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

# this seems odd, but it makes lapply() along these vectors
# possible
#' @export
`[[.s2xptr` <- function(x, i) {
  x[i]
}

#' @export
`c.s2xptr` <- function(...) {
  xptr <- new_s2xptr(NextMethod(), class(..1))
  validate_s2xptr(xptr)
  xptr
}

#' @rdname new_s2xptr
#' @export
rep.s2xptr <- function(x, ...) {
  new_s2xptr(NextMethod(), class(x))
}

# EP: this seems obsolete for R >= 4.0, as rep_len is not a generic and seems to work as rep_len
#' @rdname new_s2xptr
#' @export
rep_len.s2xptr <- function(x, ...) {
  new_s2xptr(NextMethod(), class(x))
}


#' @export
print.s2xptr <- function(x, ...) {
  cat(sprintf("<%s[%s]>\n", class(x)[1], length(x)))
  if (length(x) == 0) {
    return(invisible(x))
  }

  out <- stats::setNames(format(x, ...), names(x))
  print(out, quote = FALSE)
  invisible(x)
}
