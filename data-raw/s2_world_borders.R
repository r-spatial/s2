
library(sf)
library(rnaturalearth)

ne <- ne_countries(returnclass = "sf") %>% st_set_precision(1e7)

# write and read to ensure ring direction
temp_json <- tempfile(fileext = ".geojson")
write_sf(ne, temp_json)
ne <- read_sf(temp_json, check_ring_dir = TRUE)

# remove the south pole from antarctica
ant_ind <- which(ne$admin == "Antarctica")
ne$geometry[[ant_ind]][] <- lapply(ne$geometry[[ant_ind]], function(ply) {
  lapply(ply, function(ring) {
    is_pole <- abs(ring[, 2] + 90) < 1e-6
    ring[!is_pole, ]
  })
})

ne_wkb <- st_as_binary(ne$geometry, EWKT = TRUE) %>% wk::wkb()
s2_data_world_borders <- as.data.frame(
  tibble::tibble(
    name = ne$admin,
    continent = ne$continent,
    geometry = ne_wkb
  )
)

# check to see if this will fly
wb_s2 <- s2::s2geography(ne_wkb)

usethis::use_data(s2_data_world_borders, overwrite = TRUE)
