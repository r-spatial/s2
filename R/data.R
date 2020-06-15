
#' Low-resolution world boundaries, timezones, and cities
#'
#' Well-known binary versions of the [Natural Earth](https://www.naturalearthdata.com/)
#' low-resolution world boundaries and timezone boundaries.
#'
#' @param name The name of a country, continent, city, or `NULL`
#'   for all features.
#' @param utc_offset_min,utc_offset_max Minimum and/or maximum timezone
#'   offsets.
#'
#' @format A data.frame with columns `name` (character), and
#'   `geometry` (wk_wkb)
#' @source [Natural Earth Data](https://www.naturalearthdata.com/)
#' @examples
#' head(s2_data_countries())
#' s2_data_countries("Germany")
#' s2_data_countries("Europe")
#'
#' head(s2_data_timezones())
#' s2_data_timezones(-4)
#'
#' head(s2_data_cities())
#' s2_data_cities("Cairo")
#'
"s2_data_tbl_countries"

#' @rdname s2_data_tbl_countries
"s2_data_tbl_timezones"

#' @rdname s2_data_tbl_countries
"s2_data_tbl_cities"

#' @rdname s2_data_tbl_countries
#' @export
s2_data_countries <- function(name = NULL) {
  df <- s2::s2_data_tbl_countries
  if (is.null(name)) {
    wkb <- df$geometry
  } else {
    wkb <- structure(df$geometry[(df$name %in% name) | (df$continent %in% name)], class = "wk_wkb")
  }

  as_s2_geography(wkb)
}

#' @rdname s2_data_tbl_countries
#' @export
s2_data_timezones <- function(utc_offset_min = NULL, utc_offset_max = utc_offset_min) {
  df <- s2::s2_data_tbl_timezones
  if (is.null(utc_offset_min)) {
    wkb <- df$geometry
  } else {
    matches <- (df$utc_offset >= utc_offset_min) & (df$utc_offset <= utc_offset_max)
    wkb <- structure(df$geometry[matches], class = "wk_wkb")
  }

  as_s2_geography(wkb)
}

#' @rdname s2_data_tbl_countries
#' @export
s2_data_cities <- function(name = NULL) {
  df <- s2::s2_data_tbl_cities
  if (is.null(name)) {
    wkb <- df$geometry
  } else {
    wkb <- structure(df$geometry[(df$name %in% name)], class = "wk_wkb")
  }

  as_s2_geography(wkb)
}
