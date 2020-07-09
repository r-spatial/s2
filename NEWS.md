# s2 1.0.1

* Added layer creation options to `s2_options()`, which now uses strings
  rather than numeric codes to specify boolean operation options, geography
  construction options, and builder options (#70).
* Added `s2_rebuild()` and `s2_simplify()`, which wrap the S2 C++ `S2Builder`
  class to provide simplification and fixing of invalid geographies (#70).
* The s2 package now builds and passes the CMD check on Solaris (#66, #67).
* Renamed `s2_latlng()` to `s2_lnglat()` to keep axis order consistent
  throughout the package (#69).
* Added `s2_bounds_cap()` and `s2_bounds_rect()` to compute bounding areas
  using geographic coordinates (@edzer, #63).
* `s2_*_matrix()` predicates now efficiently use indexing to compute the 
  results of many predicate comparisons (#61).

# s2 1.0.0

This version is a complete rewrite of the former s2 CRAN package, entirely 
backwards incompatible with previous versions.
