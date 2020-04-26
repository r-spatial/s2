library(libs2)

m = rbind(c(0,0), c(1,0), c(1,1), c(0,1), c(0,0))
libs2:::s2MakePolygon(list(m));

data(nc)
g = lapply(nc_raw, unlist, recursive = FALSE)
l = lapply(g[c(1:10,10)], s2MakePolygon) #, oriented = FALSE) default: will correct ring dir
s2Intersects(l, l)
l = lapply(g[c(1:10,10)], s2MakePolygon, oriented = TRUE) # now give wrong answer: rings are not oriented!
s2Intersects(l, l)
  
#  nc = read_sf(system.file("gpkg/nc.gpkg", package="sf"), check_ring_dir = TRUE)
#  g = st_cast(st_geometry(nc), "POLYGON")
g = lapply(nc_correct, unlist, recursive = FALSE)
l = lapply(g[c(1:10,10)], s2MakePolygon, oriented = TRUE) # OK
s2Intersects(l, l)
l = lapply(g[c(1:10,10)], s2MakePolygon) # OK: will check ring dir again
s2Intersects(l, l)
  
s2BuildPolygonsLayer(l, NULL)
