
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
#' @param epsilon_east_west,epsilon_north_south Use a positive number to
#'   define the edges of a Cartesian world slightly inward from -180, -90,
#'   180, 90. This may be used to define a world outline for a projection where
#'   projecting at the extreme edges of the earth results in a non-finite value.
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
  stopifnot(is.null(s2_projection) || inherits(s2_projection, "s2_projection"))
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
  stopifnot(is.null(projection) || inherits(projection, "s2_projection"))

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
#' @importFrom wk wk_writer
#' @method wk_writer s2_geography
#' @export
wk_writer.s2_geography <- function(handleable, ...) {
  s2_geography_writer()
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
s2_hemisphere <- function(centre) {
  cap_to_polygon(centre, pi / 2)
}

#' @rdname wk_handle.s2_geography
#' @export
s2_world_plate_carree <- function(epsilon_east_west = 0, epsilon_north_south = 0) {
  s2_make_polygon(
    c(
      -180 + epsilon_east_west, 0, 180 - epsilon_east_west,
      180 - epsilon_east_west, 180 - epsilon_east_west, 0,
      -180 + epsilon_east_west, -180 + epsilon_east_west
    ),
    c(
      -90 + epsilon_north_south, -90 + epsilon_north_south,
      -90 + epsilon_north_south, 0, 90 - epsilon_north_south,
      90 - epsilon_north_south, 90 - epsilon_north_south, 0
    ),
    oriented = TRUE
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
