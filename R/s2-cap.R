

#' compute S2Cap of each geography
#' 
#' @param x object of class s2_geography
#' @export
#' @return \code{s2_cap} returns a bounding spherical cap that contains the region. 
#' For points, polylines and geometry collections the bound may not be tight.
#' @examples
#' s2_cap(s2_data_countries("Antarctica"))
#' s2_cap(s2_data_countries("Netherlands"))
#' s2_cap(s2_data_countries("Fiji"))
s2_cap <- function(x) {
  cpp_s2_cap(as_s2_geography(x))
}

#' @name s2_cap
#' @export
#' @return \code{s2_lat_lng_rect} returns a bounding latitude-longitude 
#' rectangle that contains the region.  The bound may not be tight for points, 
#' polylines and geometry collections. lng_lo values larger than lng_hi indicate 
#' regions that span the antimeridian, see the Fiji example.
#' @examples
#' s2_lat_lng_rect(s2_data_countries("Antarctica"))
#' s2_lat_lng_rect(s2_data_countries("Netherlands"))
#' s2_lat_lng_rect(s2_data_countries("Fiji"))
s2_lat_lng_rect <- function(x) {
  cpp_s2_lat_lng_rect(as_s2_geography(x))
}
