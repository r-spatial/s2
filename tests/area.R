library(s2)

u = s2_union(
   "POLYGON ((0 0, 10 0, 10 10, 0 10, 0 0))",
   "POLYGON ((5 5, 15 5, 15 15, 5 15, 5 5))",
   s2_options(snap = s2_snap_level(30))
)
s2_area(u, radius = 1)
s2_area("POLYGON ((0 0, 10 0, 10 10, 0 10, 0 0))", radius = 1) +
s2_area("POLYGON ((5 5, 15 5, 15 15, 5 15, 5 5))", radius = 1) -
s2_area("POLYGON ((5 5, 10 5, 10 15, 5 10, 5 5))", radius = 1)
s2_area("POLYGON ((0 0, 10 0, 10 10, 0 10, 0 0))", radius = 1)
s2_area("POLYGON ((5 5, 15 5, 15 15, 5 15, 5 5))", radius = 1)
s2_area("POLYGON ((5 5, 10 5, 10 15, 5 10, 5 5))", radius = 1)

df = s2_difference(
   "POLYGON ((0 0, 10 0, 10 10, 0 10, 0 0))",
   "POLYGON ((5 5, 15 5, 15 15, 5 15, 5 5))",
   s2_options(snap = s2_snap_level(30))
)
s2_area(df, radius = 1) -
  (s2_area("POLYGON ((0 0, 10 0, 10 10, 0 10, 0 0))", radius = 1) -
    s2_area("POLYGON ((5 5, 10 5, 10 15, 5 10, 5 5))", radius = 1))
