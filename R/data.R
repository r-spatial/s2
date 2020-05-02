
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
