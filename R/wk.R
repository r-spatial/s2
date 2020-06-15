
#' @importFrom wk as_wkb
#' @export
as_wkb.s2_latlng <- function(x, ..., endian = wk::wk_platform_endian()) {
  wk::new_wk_wkb(wkb_from_s2_latlng(x, endian))
}

#' @importFrom wk as_wkb
#' @export
as_wkb.s2_point <- function(x, ..., endian = wk::wk_platform_endian()) {
  # wk::new_wk_wkb(wkb_from_s2_point(x, endian))
  stop("Not implemented")
}

#' @importFrom wk as_wkb
#' @export
as_wkb.s2geography <- function(x, ..., endian = wk::wk_platform_endian()) {
  wk::new_wk_wkb(s2geography_to_wkb(x, endian))
}

#' @importFrom wk as_wkt
#' @export
as_wkt.s2geography <- function(x, ..., precision = 16, trim = TRUE) {
  wk::new_wk_wkt(s2geography_to_wkt(x, precision = precision, trim = trim))
}

#' @importFrom wk as_wkt
#' @export
as_wkt.s2_latlng <- function(x, ...,  precision = 16, trim = TRUE) {
  # wk::new_wk_wkb(wkt_from_s2_latlng(x, precision = precision, trim = trim))
  stop("Not implemented")
}

#' @importFrom wk as_wkt
#' @export
as_wkt.s2_point <- function(x, ...,  precision = 16, trim = TRUE) {
  # wk::new_wk_wkt(wkt_from_s2_point(x, precision = precision, trim = trim))
  stop("Not implemented")
}
