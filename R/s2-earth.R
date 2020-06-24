
#' Earth Constants
#'
#' According to Yoder (1995), the radius of the earth is
#' 6371.01 km. These functions are used to set the
#' default radis for functions that return a distance
#' or accept a distance as input
#' (e.g., [s2_distance()] and [s2_dwithin()]).
#'
#' @export
#'
#' @references
#' Yoder, C.F. 1995. "Astrometric and Geodetic Properties of Earth and the
#' Solar System" in Global Earth Physics, A Handbook of Physical Constants,
#' AGU Reference Shelf 1, American Geophysical Union, Table 2.
#' <https://doi.org/10.1029/RF001p0001>
#'
#' @examples
#' s2_earth_radius_meters()
#'
s2_earth_radius_meters <- function() {
  6371.01 * 1000
}
