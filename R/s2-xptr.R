
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

# data.frame() will call as.data.frame() with optional = TRUE
#' @export
as.data.frame.s2_xptr <- function(x, ..., optional = FALSE) {
  if (!optional) {
    NextMethod()
  } else {
    new_data_frame(list(x))
  }
}

# lifted from vctrs::obj_leaf()
#' @export
str.s2_xptr <- function(object, ..., indent.str = "", width = getOption("width")) {
  if (length(object) == 0) {
    cat(paste0(" ", class(object)[1], "[0]\n"))
    return(invisible(object))
  }

  # estimate possible number of elements that could be displayed
  # to avoid formatting too many
  width <- width - nchar(indent.str) - 2
  length <- min(length(object), ceiling(width / 5))
  formatted <- format(object[seq_len(length)], trim = TRUE)

  title <- paste0(" ", class(object)[1], "[1:", length(object), "]")
  cat(
    paste0(
      title,
      " ",
      strtrim(paste0(formatted, collapse = ", "), width - nchar(title)),
      "\n"
    )
  )
  invisible(object)
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
