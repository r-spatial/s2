
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
#' A modified version of [maptools'][maptools::maptools-package] `wrld_simpl` dataset
#' in WKB format. Use the [s2data_country()] helper to load an [s2polygon()] vector for
#' a given country.
#'
#' @param name The name or ISO3 identifier of a country, or `NULL`
#'   for all countries.
#'
#' @format A list with components `ISO3` (character), `NAME` (character), and
#'   `geometry` (wk_wkb)
#' @source The [mapview package][mapview::mapview-package].
#' @examples
#' world <- s2polygon(s2_wrld_simpl_wkb$geometry)
#' head(world)
#'
#' s2data_country("Germany")
#'
"s2_wrld_simpl_wkb"

#' @rdname s2_wrld_simpl_wkb
#' @export
s2data_country <- function(name = NULL) {
  df <- libs2::s2_wrld_simpl_wkb
  if (is.null(name)) {
    wkb <- df$geometry
  } else {
    wkb <- structure(df$geometry[(df$NAME %in% name) | (df$ISO3 %in% name)], class = "wk_wkb")
  }

  s2polygon(wkb, oriented = FALSE, check = TRUE)
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
