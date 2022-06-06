
#' Low-level wk filters and handlers
#'
#' @inheritParams wk::wk_handle
#' @param projection,s2_projection One of [s2_projection_plate_carree()] or
#'   [s2_projection_mercator()]
#' @param tessellate_tol,s2_tessellate_tol An angle in radians.
#'   Points will not be added if a line segment is within this
#'   distance of a point.
#' @param x_scale The maximum x value of the projection
#' @param centre The center point of the orthographic projection
#' @inheritParams as_s2_geography
#'
#' @return
#'   - `s2_projection_plate_carree()`, `s2_projection_mercator()`: An external pointer
#'     to an S2 projection.
#' @importFrom wk wk_handle
#' @export
#'
wk_handle.s2_geography <- function(handleable, handler, ...,
                                   s2_projection = s2_projection_plate_carree(),
                                   s2_tessellate_tol = Inf)  {
  stopifnot(inherits(s2_projection, "s2_projection"))
  attr(handleable, "s2_projection") <- s2_projection

  if (identical(s2_tessellate_tol, Inf)) {
    .Call(c_s2_handle_geography, handleable, wk::as_wk_handler(handler))
  } else {
    attr(handleable, "s2_tessellate_tol") <- as.double(s2_tessellate_tol)[1]
    .Call(c_s2_handle_geography_tessellated, handleable, wk::as_wk_handler(handler))
  }
}

#' @rdname wk_handle.s2_geography
#' @export
s2_geography_writer <- function(oriented = FALSE, check = TRUE,
                                projection = s2_projection_plate_carree(),
                                tessellate_tol = Inf) {
  stopifnot(inherits(projection, "s2_projection"))

  wk::new_wk_handler(
    .Call(
      c_s2_geography_writer_new,
      as.logical(oriented)[1],
      as.logical(check)[1],
      projection,
      as.double(tessellate_tol[1])
    ),
    "s2_geography_writer"
  )
}

#' @rdname wk_handle.s2_geography
#' @export
s2_trans_point <- function() {
  wk::new_wk_trans(.Call(c_s2_trans_s2_point_new))
}

#' @rdname wk_handle.s2_geography
#' @export
s2_trans_lnglat <- function() {
  wk::new_wk_trans(.Call(c_s2_trans_s2_lnglat_new))
}

#' @rdname wk_handle.s2_geography
#' @export
s2_projection_plate_carree <- function(x_scale = 180) {
  structure(
    .Call(c_s2_projection_plate_carree, as.double(x_scale)[1]),
    class = "s2_projection"
  )
}

#' @rdname wk_handle.s2_geography
#' @export
s2_projection_mercator <- function(x_scale = 20037508.3427892) {
  structure(
    .Call(c_s2_projection_mercator, as.double(x_scale)[1]),
    class = "s2_projection"
  )
}

#' @rdname wk_handle.s2_geography
#' @export
s2_projection_orthographic <- function(centre = s2_lnglat(0, 0)) {
  centre <- as_s2_lnglat(centre)
  centre <- as.matrix(centre)

  structure(
    .Call(c_s2_projection_orthographic, centre[1:2]),
    class = "s2_projection"
  )
}
