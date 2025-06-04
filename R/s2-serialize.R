s2_geography_serialize <- function(x) {
  wk::wk_handle(
    as_s2_geography(x),
    wk::wkb_writer(endian = 1L),
    s2_projection = NULL
  )
}

s2_geography_unserialize <- function(bytes) {
  wk::wk_handle(
    bytes,
    s2::s2_geography_writer(
      oriented = TRUE,
      check = FALSE,
      projection = NULL
    )
  )
}
