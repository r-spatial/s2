
library(sf)

curl::curl_download(
  "https://www.naturalearthdata.com/http//www.naturalearthdata.com/download/10m/cultural/ne_10m_time_zones.zip",
  "data-raw/timezones.zip"
)

unzip("data-raw/timezones.zip", exdir = "data-raw/timezones")

# this won't preserve topology...when this is possible with S2, this should be updated
tz <- read_sf("data-raw/timezones/ne_10m_time_zones.shp", check_ring_dir = TRUE) %>%
  st_set_crs(NA) %>%
  # don't get too close to the poles
  st_intersection(sf::st_as_sfc(wk::rct(-179.9, -89.9, 179.9, 89.9))) %>%
  st_simplify(dTolerance = 1e-2) %>%
  st_set_crs(4326)

# there's one invalid ring in feature 30 that has to get nixed
# to load into S2
tz$geometry[[30]] <- st_multipolygon(tz$geometry[[30]][1])

tz$geometry <- tz$geometry %>%
  lwgeom::st_geod_segmentize(units::set_units(1e3, "km"))

tz_wkb <- st_as_binary(tz$geometry, EWKB = TRUE)

s2_data_tbl_timezones <- as.data.frame(
  tibble::tibble(
    utc_offset = tz$zone,
    geometry = wk::wkb(tz_wkb, geodesic = TRUE)
  )
)

# make sure this will load into S2
s2 <- s2::s2_geog_from_wkb(s2_data_tbl_timezones$geometry, check = TRUE)
s2_wkb <- s2::s2_as_binary(s2)

usethis::use_data(s2_data_tbl_timezones, overwrite = TRUE)

unlink("data-raw/timezones.zip")
unlink("data-raw/timezones", recursive = TRUE)
