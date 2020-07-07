test_that("s2_cap works", {
 s2_cap(s2_data_countries("Antarctica"))
 s2_cap(s2_data_countries("Netherlands"))
 s2_cap(s2_data_countries("Fiji"))
})

test_that("s2_lat_lng_rect works", {
 s2_lat_lng_rect(s2_data_countries("Antarctica"))
 s2_lat_lng_rect(s2_data_countries("Netherlands"))
 s2_lat_lng_rect(s2_data_countries("Fiji"))
 s2_lat_lng_rect("MULTIPOINT(-179 0,179 1,-180 10)")
 s2_lat_lng_rect("LINESTRING(-179 0,179 1)")
})
