
#' S2-sf compatibility functions
#'
#' @inheritParams as_wkb
#'
#' @export
#'
s2latlng.sfc <- function(lat, ...) {
  s2latlng(sf_as_wkb_base(lat), ...)
}

#' @rdname s2latlng.sfc
#' @export
s2polyline.sfc <- function(x, ...) {
  s2polyline(sf_as_wkb_base(x), ...)
}

#' @rdname s2latlng.sfc
#' @export
s2polygon.sfc <- function(x, ...) {
  s2polygon(sf_as_wkb_base(x), ...)
}

# dynamically registered in zzz.R
st_as_sfc.s2latlng <- function(x, ...) {
  st_as_sfc_base(
    x,
    # POINT (nan nan)
    null = as.raw(
      c(0x01,
        0x01, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xf8, 0x7f,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xf8, 0x7f
      )
    )
  )
}

# dynamically registered in zzz.R
st_as_sfc.s2polyline <- function(x, ...) {
  st_as_sfc_base(
    x,
    # LINESTRING EMPTY
    null = as.raw(c(0x01, 0x02, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00))
  )
}

# dynamically registered in zzz.R
st_as_sfc.s2polygon <- function(x, ...) {
  st_as_sfc_base(
    x,
    # POLYGON EMPTY
    null = as.raw(c(0x01, 0x03, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00))
  )
}

sf_as_wkb_base <- function(x) {
  x <- sf_st_transform(x, 4326)
  wkb <- sf_st_as_binary(x, EWKB = TRUE)
  class(wkb) <- "wk_wkb"
  wkb
}

st_as_sfc_base <- function(x, null) {
  wkb <- unclass(as_wkb(x))
  wkb[vapply(wkb, is.null, logical(1))] <- list(null)
  class(wkb) <- "WKB"

  # avoids a CMD check note about sf in Suggests
  # until this can live there
  sf_st_as_sfc(wkb, EWKB = TRUE)
}

# compatibility functions to avoid a CMD check note and keep
# tests readable
sf_st_as_sfc <- function(...) getNamespace("sf")[["st_as_sfc"]](...)
sf_st_sfc <- function(...) getNamespace("sf")[["st_sfc"]](...)
sf_st_as_binary <- function(...) getNamespace("sf")[["st_as_binary"]](...)
sf_st_crs <- function(...) getNamespace("sf")[["st_crs"]](...)
sf_st_point <- function(...) getNamespace("sf")[["st_point"]](...)
sf_st_linestring <- function(...) getNamespace("sf")[["st_linestring"]](...)
sf_st_polygon <- function(...) getNamespace("sf")[["st_polygon"]](...)
sf_st_coordinates <- function(...) getNamespace("sf")[["st_coordinates"]](...)
sf_st_transform <- function(...) getNamespace("sf")[["st_transform"]](...)
