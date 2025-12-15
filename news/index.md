# Changelog

## s2 (development version)

- Implement serialization support for `s2_geography` vectors. This not
  only allows data containing s2 geography to be safely saved and
  restored across R sessions, but also enables using `s2` in parallel
  algorithms such as `mclapply()`
  ([\#283](https://github.com/r-spatial/s2/issues/283))
- Fix code to help gcc-ubsan understand the region coverer
  ([\#275](https://github.com/r-spatial/s2/issues/275))
- Inspect `S2_FORCE_BUNDLED_ABSEIL` in `conifigure`: if non-empty, any
  system install of Abseil is ignored (e.g., if using a non-standard
  compiler on a system where system Abseil is available via pkg-config)
  ([\#275](https://github.com/r-spatial/s2/issues/275))
- Disable optimization in compact_array.h that confused compilers when
  compiling with `-Wpedantic`
  ([\#275](https://github.com/r-spatial/s2/issues/275)).
- Add `cmake` to SystemReqirements. Even though this is technically
  optional (system Abseil can be used), adding to requirements helps
  some installers automatically install the dependency
  ([\#277](https://github.com/r-spatial/s2/issues/277)).

## s2 1.1.8

CRAN release: 2025-05-12

- [`s2_buffer_cells()`](https://r-spatial.github.io/s2/reference/s2_boundary.md)
  recycles `max_dist` and `min_level` arguments, allowing to specify
  these by feature ([\#264](https://github.com/r-spatial/s2/issues/264)
  and <https://github.com/r-spatial/sf/issues/2488>).
- The internal version of s2geometry is now 0.11.1
  ([\#257](https://github.com/r-spatial/s2/issues/257),
  [\#263](https://github.com/r-spatial/s2/issues/263)).
- The Abseil dependency is resolved using pkg-config where possible.
  Where this is not possible, a vendored version of Abseil will be built
  using CMake ([\#258](https://github.com/r-spatial/s2/issues/258)).

## s2 1.1.7

CRAN release: 2024-07-17

## s2 1.1.6

CRAN release: 2023-12-19

- Fix CRAN warning
  ([\#254](https://github.com/r-spatial/s2/issues/254)).

## s2 1.1.5

CRAN release: 2023-12-10

- fix compiler problem on Alpine 3.19.0
  ([\#251](https://github.com/r-spatial/s2/issues/251))

## s2 1.1.4

CRAN release: 2023-05-17

- Updated more tests to pass on a forthcoming waldo package update
  ([\#237](https://github.com/r-spatial/s2/issues/237)).

## s2 1.1.3

CRAN release: 2023-04-27

- Made a test less strict to pass tests on Alpine Linux
  ([\#218](https://github.com/r-spatial/s2/issues/218),
  [\#220](https://github.com/r-spatial/s2/issues/220)).
- Updated tests to pass on forthcoming waldo package update
  ([@hadley](https://github.com/hadley),
  [\#226](https://github.com/r-spatial/s2/issues/226)).
- Updated vendored file modifications to suppress a multi-line comment
  warning on gcc ([\#214](https://github.com/r-spatial/s2/issues/214),
  [\#227](https://github.com/r-spatial/s2/issues/227)).

## s2 1.1.2

CRAN release: 2023-01-12

- Fixed test for
  [`as.data.frame()`](https://rdrr.io/r/base/as.data.frame.html) for
  [`s2_cell()`](https://r-spatial.github.io/s2/reference/s2_cell.md) to
  comply with new wk version and the latest release of R
  ([\#207](https://github.com/r-spatial/s2/issues/207)).
- Fix unary union of an empty multipolygon
  ([\#208](https://github.com/r-spatial/s2/issues/208)).
- Added `#include <cstdint>` to an Abseil header to fix compilation with
  gcc13 ([\#209](https://github.com/r-spatial/s2/issues/209),
  [\#210](https://github.com/r-spatial/s2/issues/210)).
- Update internal Abseil to 20220623.1 LTS
  ([\#213](https://github.com/r-spatial/s2/issues/213)).

## s2 1.1.1

CRAN release: 2022-11-17

- Fix new CRAN check warnings
  ([\#202](https://github.com/r-spatial/s2/issues/202),
  [\#203](https://github.com/r-spatial/s2/issues/203)).

## s2 1.1.0

CRAN release: 2022-07-18

- Fix for s2 build on Windows with R \<= 3.6.x
  ([\#142](https://github.com/r-spatial/s2/issues/142))
- Fix for s2 build on MacOS with multiple openssl versions
  ([\#142](https://github.com/r-spatial/s2/issues/142),
  [\#145](https://github.com/r-spatial/s2/issues/145),
  [\#146](https://github.com/r-spatial/s2/issues/146))
- Fix for s2 build on 32-bit openssl
  ([\#143](https://github.com/r-spatial/s2/issues/143),
  [\#147](https://github.com/r-spatial/s2/issues/147))
- Added
  [`s2_convex_hull()`](https://r-spatial.github.io/s2/reference/s2_boundary.md)
  and
  [`s2_convex_hull_agg()`](https://r-spatial.github.io/s2/reference/s2_boundary.md)
  ([@spiry34](https://github.com/spiry34),
  [\#150](https://github.com/r-spatial/s2/issues/150),
  [\#151](https://github.com/r-spatial/s2/issues/151),
  [\#163](https://github.com/r-spatial/s2/issues/163)).
- Added `max_distance` argument to
  [`s2_closest_edges()`](https://r-spatial.github.io/s2/reference/s2_closest_feature.md),
  making distance-constrained k-nearest neighbours possible
  ([\#125](https://github.com/r-spatial/s2/issues/125),
  [\#156](https://github.com/r-spatial/s2/issues/156),
  [\#162](https://github.com/r-spatial/s2/issues/162)).
- Added a spherical
  [`s2_point_on_surface()`](https://r-spatial.github.io/s2/reference/s2_boundary.md)
  implementation for polygons
  ([@kylebutts](https://github.com/kylebutts),
  [\#152](https://github.com/r-spatial/s2/issues/152),
  [\#161](https://github.com/r-spatial/s2/issues/161))
- Added a
  [`s2_cell_union()`](https://r-spatial.github.io/s2/reference/s2_cell_union.md)
  vector class to represent cell coverings and operators to generate
  them from an s2 geography vector (e.g.,
  [`s2_covering_cell_ids()`](https://r-spatial.github.io/s2/reference/s2_cell_union_normalize.md)).
  Cell unions are useful as compact representations of spherical
  geometry and can be used like a bounding box to determine a possible
  intersection with one or more geographies
  ([\#85](https://github.com/r-spatial/s2/issues/85),
  [\#94](https://github.com/r-spatial/s2/issues/94),
  [\#164](https://github.com/r-spatial/s2/issues/164)).
- Refactored the simple features compatability layer into a standalone
  code base for potential future use in a Python adaptation
  ([\#165](https://github.com/r-spatial/s2/issues/165)).
- Migrate input and output to non-deprecated wk package handlers and
  writers ([\#101](https://github.com/r-spatial/s2/issues/101),
  [\#165](https://github.com/r-spatial/s2/issues/165),
  [\#168](https://github.com/r-spatial/s2/issues/168)).
- Make
  [`s2_union_agg()`](https://r-spatial.github.io/s2/reference/s2_boundary.md)
  more efficient using a recursive merge strategy
  ([\#103](https://github.com/r-spatial/s2/issues/103),
  [\#165](https://github.com/r-spatial/s2/issues/165)).
- Fix package build on Raspberry Pi
  ([\#169](https://github.com/r-spatial/s2/issues/169),
  [\#171](https://github.com/r-spatial/s2/issues/171)).
- Fix warning on clang14 when compiling with `-O0`
  ([\#167](https://github.com/r-spatial/s2/issues/167),
  [\#172](https://github.com/r-spatial/s2/issues/172)).
- Added
  [`s2_prepared_dwithin()`](https://r-spatial.github.io/s2/reference/s2_contains.md)
  and fixed
  [`s2_dwithin_matrix()`](https://r-spatial.github.io/s2/reference/s2_closest_feature.md)
  such that it efficiently uses the index
  ([\#157](https://github.com/r-spatial/s2/issues/157),
  [\#174](https://github.com/r-spatial/s2/issues/174)).
- Updated
  [`s2_lnglat()`](https://r-spatial.github.io/s2/reference/s2_lnglat.md)
  and
  [`s2_point()`](https://r-spatial.github.io/s2/reference/s2_point.md)
  to use
  [`wk::xy()`](https://paleolimbot.github.io/wk/reference/xy.html) (a
  record-style vctr) to represent point coordinates. This is much faster
  than the previous representation which relied on
  [`list()`](https://rdrr.io/r/base/list.html) of external pointers
  ([\#181](https://github.com/r-spatial/s2/issues/181),
  [\#159](https://github.com/r-spatial/s2/issues/159)).
- Added arguments `planar` and `tessellate_tol_m` to
  [`s2_as_text()`](https://r-spatial.github.io/s2/reference/s2_geog_point.md),
  [`s2_as_binary()`](https://r-spatial.github.io/s2/reference/s2_geog_point.md).
  Use `planar = TRUE` and set `tessellate_tol_m` to the maximum error
  for your use-case to automatically subdivide edges to preserve or
  “straight” lines in Plate carree projection on import
  ([\#182](https://github.com/r-spatial/s2/issues/182)).
- Added arguments `planar` and `tessellate_tol_m` to
  [`s2_geog_from_text()`](https://r-spatial.github.io/s2/reference/s2_geog_point.md),
  and
  [`s2_geog_from_wkb()`](https://r-spatial.github.io/s2/reference/s2_geog_point.md).
  Use `planar = TRUE` and set `tessellate_tol_m` to the maximum error
  for your use-case to automatically subdivide edges to ensure or
  “straight” lines in Plate carree projection on export
  ([\#182](https://github.com/r-spatial/s2/issues/182)).

## s2 1.0.7

CRAN release: 2021-09-28

- Update the internal copy of s2geometry to use updated Abseil, fixing a
  compiler warning on gcc-11
  ([\#79](https://github.com/r-spatial/s2/issues/79),
  [\#134](https://github.com/r-spatial/s2/issues/134)).

## s2 1.0.6

CRAN release: 2021-06-17

- Added support for `STRICT_R_HEADERS`
  ([@eddelbuettel](https://github.com/eddelbuettel),
  [\#118](https://github.com/r-spatial/s2/issues/118)).
- Fixed a bug where the result of
  [`s2_centroid_agg()`](https://r-spatial.github.io/s2/reference/s2_boundary.md)
  did not behave like a normal point in distance calculations
  ([\#119](https://github.com/r-spatial/s2/issues/119),
  [\#121](https://github.com/r-spatial/s2/issues/121)).
- Fixed a Windows UCRT check failure and updated openssl linking
  ([@jeroen](https://github.com/jeroen),
  [\#122](https://github.com/r-spatial/s2/issues/122)).

## s2 1.0.5

CRAN release: 2021-06-01

- Added `s2_projection_filter()` and `s2_unprojection_filter()` to
  expose the S2 edge tessellator, which can be used to make Cartesian or
  great circle assumptions of line segments explicit by adding points
  where necessary ([\#115](https://github.com/r-spatial/s2/issues/115)).
- Added an
  [`s2_cell()`](https://r-spatial.github.io/s2/reference/s2_cell.md)
  vector class to expose a subset of the S2 indexing system to R users
  ([\#85](https://github.com/r-spatial/s2/issues/85),
  [\#114](https://github.com/r-spatial/s2/issues/114)).
- Added
  [`s2_closest_edges()`](https://r-spatial.github.io/s2/reference/s2_closest_feature.md)
  to make k-nearest neighbours calculation possible on the sphere
  ([\#111](https://github.com/r-spatial/s2/issues/111),
  [\#112](https://github.com/r-spatial/s2/issues/112)).
- Added
  [`s2_interpolate()`](https://r-spatial.github.io/s2/reference/s2_interpolate.md),
  [`s2_interpolate_normalized()`](https://r-spatial.github.io/s2/reference/s2_interpolate.md),
  [`s2_project()`](https://r-spatial.github.io/s2/reference/s2_interpolate.md),
  and
  [`s2_project_normalized()`](https://r-spatial.github.io/s2/reference/s2_interpolate.md)
  to provide linear referencing support on the sphere
  ([\#96](https://github.com/r-spatial/s2/issues/96),
  [\#110](https://github.com/r-spatial/s2/issues/110)).
- Fixed import of empty points from WKB
  ([\#109](https://github.com/r-spatial/s2/issues/109)).
- Added argument `dimensions` to
  [`s2_options()`](https://r-spatial.github.io/s2/reference/s2_options.md)
  to constrain the output dimensions of a boolean or rebuild operation
  ([\#105](https://github.com/r-spatial/s2/issues/105),
  [\#104](https://github.com/r-spatial/s2/issues/104),
  [\#110](https://github.com/r-spatial/s2/issues/110)).
- Added
  [`s2_is_valid()`](https://r-spatial.github.io/s2/reference/s2_is_collection.md)
  and
  [`s2_is_valid_detail()`](https://r-spatial.github.io/s2/reference/s2_is_collection.md)
  to help find invalid spherical geometries when importing data into S2
  ([\#100](https://github.com/r-spatial/s2/issues/100)).
- Improved error messages when importing and processing data such that
  errors can be debugged more readily
  ([\#100](https://github.com/r-spatial/s2/issues/100),
  [\#98](https://github.com/r-spatial/s2/issues/98)).
- The unary version of
  [`s2_union()`](https://r-spatial.github.io/s2/reference/s2_boundary.md)
  can now handle MULTIPOLYGON geometries with overlapping rings in
  addition to other invalid polygons.
  [`s2_union()`](https://r-spatial.github.io/s2/reference/s2_boundary.md)
  can now sanitize almost any input to be valid spherical geometry with
  minimal modification
  ([\#100](https://github.com/r-spatial/s2/issues/100),
  [\#99](https://github.com/r-spatial/s2/issues/99)).
- Renamed the existing implementation of
  [`s2_union_agg()`](https://r-spatial.github.io/s2/reference/s2_boundary.md)
  to
  [`s2_coverage_union_agg()`](https://r-spatial.github.io/s2/reference/s2_boundary.md)
  to make clear that the function only works when the individual
  geometries do not have overlapping interiors.
  [`s2_union_agg()`](https://r-spatial.github.io/s2/reference/s2_boundary.md)
  was replaced with a true aggregate union that can handle unions of
  most geometries ([\#100](https://github.com/r-spatial/s2/issues/100),
  [\#97](https://github.com/r-spatial/s2/issues/97)).
- Added
  [`s2_rebuild_agg()`](https://r-spatial.github.io/s2/reference/s2_boundary.md)
  to match
  [`s2_union_agg()`](https://r-spatial.github.io/s2/reference/s2_boundary.md).
  Like
  [`s2_rebuild()`](https://r-spatial.github.io/s2/reference/s2_boundary.md),
  [`s2_rebuild_agg()`](https://r-spatial.github.io/s2/reference/s2_boundary.md)
  collects the edges in the input and builds them into a feature,
  optionally snapping or simplifying vertices in the process
  ([\#100](https://github.com/r-spatial/s2/issues/100)).

## s2 1.0.4

CRAN release: 2021-01-05

- Fixed errors that resulted from compilation on clang 12.2
  ([\#88](https://github.com/r-spatial/s2/issues/88),
  [\#89](https://github.com/r-spatial/s2/issues/89)).

## s2 1.0.3

CRAN release: 2020-10-14

- Fixed CRAN check errors
  ([\#80](https://github.com/r-spatial/s2/issues/80)).

## s2 1.0.2

CRAN release: 2020-08-03

- Fixed CRAN check errors
  ([\#71](https://github.com/r-spatial/s2/issues/71),
  [\#75](https://github.com/r-spatial/s2/issues/75),
  [\#72](https://github.com/r-spatial/s2/issues/72)).

## s2 1.0.1

CRAN release: 2020-07-13

- Added layer creation options to
  [`s2_options()`](https://r-spatial.github.io/s2/reference/s2_options.md),
  which now uses strings rather than numeric codes to specify boolean
  operation options, geography construction options, and builder options
  ([\#70](https://github.com/r-spatial/s2/issues/70)).
- Added
  [`s2_rebuild()`](https://r-spatial.github.io/s2/reference/s2_boundary.md)
  and
  [`s2_simplify()`](https://r-spatial.github.io/s2/reference/s2_boundary.md),
  which wrap the S2 C++ `S2Builder` class to provide simplification and
  fixing of invalid geographies
  ([\#70](https://github.com/r-spatial/s2/issues/70)).
- The s2 package now builds and passes the CMD check on Solaris
  ([\#66](https://github.com/r-spatial/s2/issues/66),
  [\#67](https://github.com/r-spatial/s2/issues/67)).
- Renamed `s2_latlng()` to
  [`s2_lnglat()`](https://r-spatial.github.io/s2/reference/s2_lnglat.md)
  to keep axis order consistent throughout the package
  ([\#69](https://github.com/r-spatial/s2/issues/69)).
- Added
  [`s2_bounds_cap()`](https://r-spatial.github.io/s2/reference/s2_bounds_cap.md)
  and
  [`s2_bounds_rect()`](https://r-spatial.github.io/s2/reference/s2_bounds_cap.md)
  to compute bounding areas using geographic coordinates
  ([@edzer](https://github.com/edzer),
  [\#63](https://github.com/r-spatial/s2/issues/63)).
- `s2_*_matrix()` predicates now efficiently use indexing to compute the
  results of many predicate comparisons
  ([\#61](https://github.com/r-spatial/s2/issues/61)).

## s2 1.0.0

CRAN release: 2020-06-27

This version is a complete rewrite of the former s2 CRAN package,
entirely backwards incompatible with previous versions.
