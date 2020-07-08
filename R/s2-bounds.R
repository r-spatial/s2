
#' Compute feature-wise and aggregate bounds
#'
#' [s2_bounds_rect()] returns a bounding latitude-longitude
#' rectangle that contains the region; [s2_bounds_cap()] returns a bounding circle
#' represented by a centre point (lat, lng) and an angle. The bound may not be tight
#' for points, polylines and geometry collections. The rectangle returned may depend on
#' the order of points or polylines. `lng_lo` values larger than `lng_hi` indicate
#' regions that span the antimeridian, see the Fiji example.
#'
#' @inheritParams s2_is_collection
#' @export
#' @return Both functions return a `data.frame`:
#'
#' - [s2_bounds_rect()]: Columns `minlng`, `minlat`, `maxlng`, `maxlat` (degrees)
#' - [s2_bounds_cap()]: Columns `lng`, `lat`, `angle` (degrees)
#'
#' @examples
#' s2_bounds_cap(s2_data_countries("Antarctica"))
#' s2_bounds_cap(s2_data_countries("Netherlands"))
#' s2_bounds_cap(s2_data_countries("Fiji"))
#'
#' s2_bounds_rect(s2_data_countries("Antarctica"))
#' s2_bounds_rect(s2_data_countries("Netherlands"))
#' s2_bounds_rect(s2_data_countries("Fiji"))
#'
s2_bounds_cap <- function(x) {
  cpp_s2_bounds_cap(as_s2_geography(x))
}

#' @rdname s2_bounds_cap
#' @export
s2_bounds_rect <- function(x) {
  cpp_s2_bounds_rect(as_s2_geography(x))
}
