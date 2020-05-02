
library(sf)

# data(wrld_simpl, package = "maptools")
# sf_wrld_simpl <- st_as_sf(wrld_simpl)
# sf_wrld_simpl %>% dplyr::select(ISO3, NAME) %>% write_sf("data-raw/wrld.geojson")

# # do a write/read to make sure ring directions are OK
# sf_wrld_simpl <- read_sf("data-raw/wrld.geojson", check_ring_dir = TRUE)
# sf_wrld_simpl_polygon <- st_cast(sf_wrld_simpl, "POLYGON")
#
# # don't use the tiny polygons
# sf_wrld_simpl_polygon <- sf_wrld_simpl_polygon[st_area(sf_wrld_simpl_polygon) > units::set_units(1e8, "m2"), ]
#
# # write so QGIS can do validity check
# write_sf(sf_wrld_simpl_polygon, "data-raw/wrld_simpler.geojson")

# (edited wrld_simpler.geojson so that countries are all valid polygons)

sf_wrld_simpl_polygon <- read_sf("data-raw/wrld_simpler.geojson")

wrld_simpl_wkb <- sf:::CPL_write_wkb(sf_wrld_simpl_polygon$geometry, EWKB = TRUE)
class(wrld_simpl_wkb) <- "wk_wkb"

# make sure all polygons can be imported
read_error <- vapply(
  wrld_simpl_wkb,
  function(x) {
    x <- list(x)
    class(x) <- "wk_wkb"
    tryCatch({
      libs2::s2polygon(x)
      NA_character_
    }, error = function(e) {
      paste0(as.character(e), collapse = "\n")
    })
  },
  character(1)
)

bad <- sf_wrld_simpl_polygon[!is.na(read_error), ]
bad$error <- read_error[!is.na(read_error)]

stopifnot(all(is.na(read_error)))

sf_wrld_simpl_tbl <- st_set_geometry(sf_wrld_simpl_polygon, NULL)
s2_wrld_simpl_wkb <-  as.list(dplyr::select(sf_wrld_simpl_tbl, ISO3, NAME))
s2_wrld_simpl_wkb$geometry <- wrld_simpl_wkb

usethis::use_data(s2_wrld_simpl_wkb, overwrite = TRUE)
