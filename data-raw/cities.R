
library(sf)
library(rnaturalearth)

curl::curl_download(
  "https://www.naturalearthdata.com/http//www.naturalearthdata.com/download/110m/cultural/ne_110m_populated_places.zip",
  "data-raw/cities.zip"
)
unzip("data-raw/cities.zip", exdir = "data-raw/cities")

sf <- read_sf("data-raw/cities/ne_110m_populated_places.shp")

s2_data_tbl_cities <- as.data.frame(
  tibble::tibble(
    name = sf$NAMEASCII,
    population = sf$POP_MIN,
    geometry = wk::wkb(st_as_binary(sf$geometry, EKWB = TRUE), geodesic = NA)
  )
)

usethis::use_data(s2_data_tbl_cities, overwrite = TRUE)
unlink("data-raw/cities.zip")
unlink("data-raw/cities", recursive = TRUE)
