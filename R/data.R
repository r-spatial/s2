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
