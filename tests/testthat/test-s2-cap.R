test_that("s2_cap works", {
 s2_cap(s2_data_countries("Antarctica"))
 s2_cap(s2_data_countries("Netherlands"))
 s2_cap(s2_data_countries("Fiji"))
})

test_that("s2_lat_lng_rect works", {
 s2_lat_lng_rect(s2_data_countries("Antarctica"))
 s2_lat_lng_rect(s2_data_countries("Netherlands"))
 s2_lat_lng_rect(s2_data_countries("Fiji"))
})
