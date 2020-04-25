library(libs2)

m = rbind(c(0,0), c(1,0), c(1,1), c(0,1), c(0,0))
libs2:::MakeS2Shape(list(m));

library(sf)
nc = read_sf(system.file("gpkg/nc.gpkg", package="sf"), check_ring_dir = FALSE)
g = st_cast(st_geometry(nc), "POLYGON")
l = lapply(g[c(1:10,10)], libs2:::MakeS2Shape) #, oriented = FALSE) default: will correct ring dir
libs2:::Intersects(l)
l = lapply(g[c(1:10,10)], libs2:::MakeS2Shape, oriented = TRUE) # now give wrong answer: rings are not oriented!
libs2:::Intersects(l)

nc = read_sf(system.file("gpkg/nc.gpkg", package="sf"), check_ring_dir = TRUE)
g = st_cast(st_geometry(nc), "POLYGON")
l = lapply(g[c(1:10,10)], libs2:::MakeS2Shape, oriented = TRUE) # OK
libs2:::Intersects(l)
l = lapply(g[c(1:10,10)], libs2:::MakeS2Shape) # OK: will check ring dir again
libs2:::Intersects(l)

libs2:::BuildPolygonsLayer(l, NULL)
