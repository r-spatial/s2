
sf_nc <- sf::read_sf(system.file("shape/nc.shp", package="sf"), check_ring_dir = TRUE)
sf_nc_polygon <- sf::st_cast(sf_nc, "POLYGON")

nc_wkb <- sf:::CPL_write_wkb(sf_nc_polygon$geometry, EWKB = TRUE)
class(nc_wkb) <- "wk_wkb"
nc_tbl <- sf::st_set_geometry(sf_nc, NULL)

s2_nc_wkb <- as.list(dplyr::select(tibble::as_tibble(nc_tbl), CNTY_ID, NAME))
s2_nc_wkb$geometry <- nc_wkb

usethis::use_data(s2_nc_wkb, overwrite = TRUE)
