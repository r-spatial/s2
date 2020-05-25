library(libs2)
s2_area(
  s2_union(
   "POLYGON ((0 0, 10 0, 10 10, 0 10, 0 0))",
   "POLYGON ((5 5, 15 5, 15 15, 5 15, 5 5))"
  ), radius = 1
)
s2_area("POLYGON ((0 0, 10 0, 10 10, 0 10, 0 0))", radius = 1) +
s2_area("POLYGON ((5 5, 15 5, 15 15, 5 15, 5 5))", radius = 1) -
s2_area("POLYGON ((5 5, 10 5, 10 15, 5 10, 5 5))", radius = 1)
s2_area("POLYGON ((0 0, 10 0, 10 10, 0 10, 0 0))", radius = 1)
s2_area("POLYGON ((5 5, 15 5, 15 15, 5 15, 5 5))", radius = 1)
s2_area("POLYGON ((5 5, 10 5, 10 15, 5 10, 5 5))", radius = 1)
