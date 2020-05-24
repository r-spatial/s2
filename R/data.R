
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
#' head(s2data_countries())
#' s2data_countries("Germany")
#' s2data_countries("Europe")
#'
#' head(s2data_timezones())
#' s2data_timezones(-4)
#'
#' head(s2data_cities())
#' s2data_cities("Cairo")
#'
"s2_data_world_borders"

#' @rdname s2_data_world_borders
"s2_data_timezones"

#' @rdname s2_data_world_borders
"s2_data_cities"

#' @rdname s2_data_world_borders
#' @export
s2data_countries <- function(name = NULL) {
  df <- libs2::s2_data_world_borders
  if (is.null(name)) {
    wkb <- df$geometry
  } else {
    wkb <- structure(df$geometry[(df$name %in% name) | (df$continent %in% name)], class = "wk_wkb")
  }

  s2geography(wkb)
}

#' @rdname s2_data_world_borders
#' @export
s2data_timezones <- function(utc_offset_min = NULL, utc_offset_max = utc_offset_min) {
  df <- libs2::s2_data_timezones
  if (is.null(utc_offset_min)) {
    wkb <- df$geometry
  } else {
    matches <- (df$utc_offset >= utc_offset_min) & (df$utc_offset <= utc_offset_max)
    wkb <- structure(df$geometry[matches], class = "wk_wkb")
  }

  s2geography(wkb)
}

#' @rdname s2_data_world_borders
#' @export
s2data_cities <- function(name = NULL) {
  df <- libs2::s2_data_cities
  if (is.null(name)) {
    wkb <- df$geometry
  } else {
    wkb <- structure(df$geometry[(df$name %in% name)], class = "wk_wkb")
  }

  s2geography(wkb)
}

#' Geometry of the nc dataset of package sf
#'
#' The raw output of st_geometry(nc) when read with package sf
#'
#' @format A list with sfg objects
#' @source \url{http://r-spatial.github.io/sf/}
#' @examples
#' # Created with:
#' # library(sf)
#' # read_sf(system.file("gpkg/nc.gpkg", package="sf"), check_ring_dir = TRUE) %>%
#' # 	st_geometry() -> nc_correct
"nc_correct"

#' Geometry of the nc dataset of package sf
#'
#' The raw output of st_geometry(nc) when read with package sf
#'
#' @format A list with sfg objects
#' @source \url{http://r-spatial.github.io/sf/}
#' @examples
#' # Created with:
#' # library(sf)
#' # read_sf(system.file("gpkg/nc.gpkg", package="sf"), check_ring_dir = FALSE) %>%
#' #	st_geometry() -> nc_raw
"nc_raw"
