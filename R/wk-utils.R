
#' Low-level wk filters and handlers
#'
#' @inheritParams wk::wk_handle
#' @param projection One of [s2_projection_plate_carree()] or
#'   [s2_projection_mercator()]
#' @param tessellate_tol An angle in radians. Points will not be added
#'   if a line segment is within this distance of a point.
#'
#' @return
#'   - `s2_unprojection_filter()`, `s2_projection_filter()`: A `new_wk_handler()`
#'   - `s2_projection_plate_carree()`, `s2_projection_mercator()`: An external pointer
#'     to an S2 projection.
#' @export
#'
#' @examples
#' library(wk)
#'
#' # simple conversion of individual coordinates *to* unit sphere
#' # space
#' wk_handle(
#'   wkt("LINESTRING (0 0, 0 45, -60 45)"),
#'   s2_unprojection_filter(wkt_format_handler(5))
#' )
#'
#' # simple conversion of individual coordinates *from* unit sphere
#' # space
#' wk_handle(
#'   wkt("LINESTRING Z (1 0 0, 0.7071 0 0.7071, 0.3536 -0.6124 0.7071)"),
#'   s2_projection_filter(wkt_format_handler(5))
#' )
#'
#' # use tessellate_tol to force points to be added to an edge
#' # unprojection will ensure an edge maintains its cartesian
#' # assumption when transformed to the unit sphere
#' # (i.e., what you probably want when importing a geography)
#' wk_handle(
#'   wkt("LINESTRING (0 0, 0 45, -60 45)"),
#'   s2_unprojection_filter(wkt_format_handler(5), tessellate_tol = 0.001)
#' )
#'
#' # projection will ensure an edge maintains its geodesic
#' # assumption when transformed to projected space
#' # (i.e., what you probably want when exporting a geography)
#' wk_handle(
#'   wkt("LINESTRING Z (1 0 0, 0.7071 0 0.7071, 0.3536 -0.6124 0.7071)"),
#'   s2_projection_filter(wkt_format_handler(5), tessellate_tol = 0.001)
#' )
#'
s2_unprojection_filter <- function(handler, projection = s2_projection_plate_carree(),
                                   tessellate_tol = Inf) {
  wk::new_wk_handler(
    .Call(c_s2_coord_filter_new, handler, projection, TRUE, tessellate_tol),
    subclass = "s2_coord_filter"
  )
}

#' @rdname s2_unprojection_filter
#' @export
s2_projection_filter <- function(handler, projection = s2_projection_plate_carree(),
                                 tessellate_tol = Inf) {
  wk::new_wk_handler(
    .Call(c_s2_coord_filter_new, handler, projection, FALSE, tessellate_tol),
    subclass = "s2_coord_filter"
  )
}

#' @rdname s2_unprojection_filter
#' @export
s2_projection_plate_carree <- function() {
  .Call(c_s2_projection_plate_carree)
}

#' @rdname s2_unprojection_filter
#' @export
s2_projection_mercator <- function() {
  .Call(c_s2_projection_mercator)
}
