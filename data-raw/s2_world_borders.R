
library(sf)
library(rnaturalearth)

ne <- ne_countries(returnclass = "sf") %>% st_set_precision(1e7)
ne_wkb <- st_as_binary(ne$geometry, EWKT = TRUE) %>% wk::wkb()
s2_data_world_borders <- as.data.frame(tibble::tibble(name = ne$admin, geometry = ne_wkb))

# check to see if this will fly in libs2
wb_s2 <- libs2::s2geography(ne_wkb)

usethis::use_data(s2_data_world_borders, overwrite = TRUE)
