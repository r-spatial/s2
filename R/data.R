
#' Geometry of the nc dataset of package sf
#'
#' A subset of [sf's][sf::sf] North Carolina dataset in WKB format.
#'
#' @format A list with components `CNTY_ID` (integer), `NAME` (character), and
#'   `geometry` (wk_wkb)
#' @source <http://r-spatial.github.io/sf/>
#' @examples
#' nc <- s2polygon(s2_nc_wkb$geometry)
#' head(nc)
#'
"s2_nc_wkb"

#' Low-resolution world boundaries
#'
#' A well-known binary version of the [Natural Earth](https://www.naturalearthdata.com/)
#' low-resolution world boundaries.
#'
#' @param name The name or of a country, or `NULL`
#'   for all countries.
#'
#' @format A data.frame with columns `name` (character), and
#'   `geometry` (wk_wkb)
#' @source [rnaturalearth::ne_countries()]
#' @examples
#' head(s2data_countries())
#' s2data_countries("Germany")
#' s2data_countries("Canada")
#'
"s2_data_world_borders"

#' @rdname s2_data_world_borders
#' @export
s2data_countries <- function(name = NULL) {
  df <- libs2::s2_data_world_borders
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
