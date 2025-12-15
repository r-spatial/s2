# Package index

## Geography Constructors and Exporters

Convert objects to and from geography vectors

- [`s2_lnglat()`](https://r-spatial.github.io/s2/reference/s2_lnglat.md)
  [`as_s2_lnglat()`](https://r-spatial.github.io/s2/reference/s2_lnglat.md)
  : Create an S2 LngLat Vector
- [`s2_point()`](https://r-spatial.github.io/s2/reference/s2_point.md)
  [`s2_point_crs()`](https://r-spatial.github.io/s2/reference/s2_point.md)
  [`as_s2_point()`](https://r-spatial.github.io/s2/reference/s2_point.md)
  : Create an S2 Point Vector
- [`as_s2_geography()`](https://r-spatial.github.io/s2/reference/as_s2_geography.md)
  [`s2_geography()`](https://r-spatial.github.io/s2/reference/as_s2_geography.md)
  [`as_wkb(`*`<s2_geography>`*`)`](https://r-spatial.github.io/s2/reference/as_s2_geography.md)
  [`as_wkt(`*`<s2_geography>`*`)`](https://r-spatial.github.io/s2/reference/as_s2_geography.md)
  : Create an S2 Geography Vector
- [`s2_geog_point()`](https://r-spatial.github.io/s2/reference/s2_geog_point.md)
  [`s2_make_line()`](https://r-spatial.github.io/s2/reference/s2_geog_point.md)
  [`s2_make_polygon()`](https://r-spatial.github.io/s2/reference/s2_geog_point.md)
  [`s2_geog_from_text()`](https://r-spatial.github.io/s2/reference/s2_geog_point.md)
  [`s2_geog_from_wkb()`](https://r-spatial.github.io/s2/reference/s2_geog_point.md)
  [`s2_as_text()`](https://r-spatial.github.io/s2/reference/s2_geog_point.md)
  [`s2_as_binary()`](https://r-spatial.github.io/s2/reference/s2_geog_point.md)
  [`s2_tessellate_tol_default()`](https://r-spatial.github.io/s2/reference/s2_geog_point.md)
  : Create and Format Geography Vectors

## Geography Transformations

Functions that operate on geography vectors and return geography vectors

- [`s2_boundary()`](https://r-spatial.github.io/s2/reference/s2_boundary.md)
  [`s2_centroid()`](https://r-spatial.github.io/s2/reference/s2_boundary.md)
  [`s2_closest_point()`](https://r-spatial.github.io/s2/reference/s2_boundary.md)
  [`s2_minimum_clearance_line_between()`](https://r-spatial.github.io/s2/reference/s2_boundary.md)
  [`s2_difference()`](https://r-spatial.github.io/s2/reference/s2_boundary.md)
  [`s2_sym_difference()`](https://r-spatial.github.io/s2/reference/s2_boundary.md)
  [`s2_intersection()`](https://r-spatial.github.io/s2/reference/s2_boundary.md)
  [`s2_union()`](https://r-spatial.github.io/s2/reference/s2_boundary.md)
  [`s2_snap_to_grid()`](https://r-spatial.github.io/s2/reference/s2_boundary.md)
  [`s2_simplify()`](https://r-spatial.github.io/s2/reference/s2_boundary.md)
  [`s2_rebuild()`](https://r-spatial.github.io/s2/reference/s2_boundary.md)
  [`s2_buffer_cells()`](https://r-spatial.github.io/s2/reference/s2_boundary.md)
  [`s2_convex_hull()`](https://r-spatial.github.io/s2/reference/s2_boundary.md)
  [`s2_centroid_agg()`](https://r-spatial.github.io/s2/reference/s2_boundary.md)
  [`s2_coverage_union_agg()`](https://r-spatial.github.io/s2/reference/s2_boundary.md)
  [`s2_rebuild_agg()`](https://r-spatial.github.io/s2/reference/s2_boundary.md)
  [`s2_union_agg()`](https://r-spatial.github.io/s2/reference/s2_boundary.md)
  [`s2_convex_hull_agg()`](https://r-spatial.github.io/s2/reference/s2_boundary.md)
  [`s2_point_on_surface()`](https://r-spatial.github.io/s2/reference/s2_boundary.md)
  : S2 Geography Transformations

## Binary Geography Predicates

Functions that operate two geography vectors and return a logical vector

- [`s2_contains()`](https://r-spatial.github.io/s2/reference/s2_contains.md)
  [`s2_within()`](https://r-spatial.github.io/s2/reference/s2_contains.md)
  [`s2_covered_by()`](https://r-spatial.github.io/s2/reference/s2_contains.md)
  [`s2_covers()`](https://r-spatial.github.io/s2/reference/s2_contains.md)
  [`s2_disjoint()`](https://r-spatial.github.io/s2/reference/s2_contains.md)
  [`s2_intersects()`](https://r-spatial.github.io/s2/reference/s2_contains.md)
  [`s2_equals()`](https://r-spatial.github.io/s2/reference/s2_contains.md)
  [`s2_intersects_box()`](https://r-spatial.github.io/s2/reference/s2_contains.md)
  [`s2_touches()`](https://r-spatial.github.io/s2/reference/s2_contains.md)
  [`s2_dwithin()`](https://r-spatial.github.io/s2/reference/s2_contains.md)
  [`s2_prepared_dwithin()`](https://r-spatial.github.io/s2/reference/s2_contains.md)
  : S2 Geography Predicates

## Geography Accessors

Functions that operate one or more geography vectors and return a vector
of values

- [`s2_is_collection()`](https://r-spatial.github.io/s2/reference/s2_is_collection.md)
  [`s2_is_valid()`](https://r-spatial.github.io/s2/reference/s2_is_collection.md)
  [`s2_is_valid_detail()`](https://r-spatial.github.io/s2/reference/s2_is_collection.md)
  [`s2_dimension()`](https://r-spatial.github.io/s2/reference/s2_is_collection.md)
  [`s2_num_points()`](https://r-spatial.github.io/s2/reference/s2_is_collection.md)
  [`s2_is_empty()`](https://r-spatial.github.io/s2/reference/s2_is_collection.md)
  [`s2_area()`](https://r-spatial.github.io/s2/reference/s2_is_collection.md)
  [`s2_length()`](https://r-spatial.github.io/s2/reference/s2_is_collection.md)
  [`s2_perimeter()`](https://r-spatial.github.io/s2/reference/s2_is_collection.md)
  [`s2_x()`](https://r-spatial.github.io/s2/reference/s2_is_collection.md)
  [`s2_y()`](https://r-spatial.github.io/s2/reference/s2_is_collection.md)
  [`s2_distance()`](https://r-spatial.github.io/s2/reference/s2_is_collection.md)
  [`s2_max_distance()`](https://r-spatial.github.io/s2/reference/s2_is_collection.md)
  : S2 Geography Accessors
- [`s2_bounds_cap()`](https://r-spatial.github.io/s2/reference/s2_bounds_cap.md)
  [`s2_bounds_rect()`](https://r-spatial.github.io/s2/reference/s2_bounds_cap.md)
  : Compute feature-wise and aggregate bounds

## Matrix Functions

These functions return various relationships between two geography
vectors

- [`s2_closest_feature()`](https://r-spatial.github.io/s2/reference/s2_closest_feature.md)
  [`s2_closest_edges()`](https://r-spatial.github.io/s2/reference/s2_closest_feature.md)
  [`s2_farthest_feature()`](https://r-spatial.github.io/s2/reference/s2_closest_feature.md)
  [`s2_distance_matrix()`](https://r-spatial.github.io/s2/reference/s2_closest_feature.md)
  [`s2_max_distance_matrix()`](https://r-spatial.github.io/s2/reference/s2_closest_feature.md)
  [`s2_contains_matrix()`](https://r-spatial.github.io/s2/reference/s2_closest_feature.md)
  [`s2_within_matrix()`](https://r-spatial.github.io/s2/reference/s2_closest_feature.md)
  [`s2_covers_matrix()`](https://r-spatial.github.io/s2/reference/s2_closest_feature.md)
  [`s2_covered_by_matrix()`](https://r-spatial.github.io/s2/reference/s2_closest_feature.md)
  [`s2_intersects_matrix()`](https://r-spatial.github.io/s2/reference/s2_closest_feature.md)
  [`s2_disjoint_matrix()`](https://r-spatial.github.io/s2/reference/s2_closest_feature.md)
  [`s2_equals_matrix()`](https://r-spatial.github.io/s2/reference/s2_closest_feature.md)
  [`s2_touches_matrix()`](https://r-spatial.github.io/s2/reference/s2_closest_feature.md)
  [`s2_dwithin_matrix()`](https://r-spatial.github.io/s2/reference/s2_closest_feature.md)
  [`s2_may_intersect_matrix()`](https://r-spatial.github.io/s2/reference/s2_closest_feature.md)
  : Matrix Functions

## Linear Referencing

- [`s2_project()`](https://r-spatial.github.io/s2/reference/s2_interpolate.md)
  [`s2_project_normalized()`](https://r-spatial.github.io/s2/reference/s2_interpolate.md)
  [`s2_interpolate()`](https://r-spatial.github.io/s2/reference/s2_interpolate.md)
  [`s2_interpolate_normalized()`](https://r-spatial.github.io/s2/reference/s2_interpolate.md)
  : Linear referencing

## S2 Cell Utilities

- [`s2_cell_union()`](https://r-spatial.github.io/s2/reference/s2_cell_union.md)
  [`as_s2_geography(`*`<s2_cell_union>`*`)`](https://r-spatial.github.io/s2/reference/s2_cell_union.md)
  [`as_s2_cell_union()`](https://r-spatial.github.io/s2/reference/s2_cell_union.md)
  : Create S2 Cell Union vectors
- [`s2_cell_union_normalize()`](https://r-spatial.github.io/s2/reference/s2_cell_union_normalize.md)
  [`s2_cell_union_contains()`](https://r-spatial.github.io/s2/reference/s2_cell_union_normalize.md)
  [`s2_cell_union_intersects()`](https://r-spatial.github.io/s2/reference/s2_cell_union_normalize.md)
  [`s2_cell_union_intersection()`](https://r-spatial.github.io/s2/reference/s2_cell_union_normalize.md)
  [`s2_cell_union_union()`](https://r-spatial.github.io/s2/reference/s2_cell_union_normalize.md)
  [`s2_cell_union_difference()`](https://r-spatial.github.io/s2/reference/s2_cell_union_normalize.md)
  [`s2_covering_cell_ids()`](https://r-spatial.github.io/s2/reference/s2_cell_union_normalize.md)
  [`s2_covering_cell_ids_agg()`](https://r-spatial.github.io/s2/reference/s2_cell_union_normalize.md)
  : S2 cell union operators
- [`s2_cell()`](https://r-spatial.github.io/s2/reference/s2_cell.md)
  [`s2_cell_sentinel()`](https://r-spatial.github.io/s2/reference/s2_cell.md)
  [`s2_cell_invalid()`](https://r-spatial.github.io/s2/reference/s2_cell.md)
  [`as_s2_cell()`](https://r-spatial.github.io/s2/reference/s2_cell.md)
  [`new_s2_cell()`](https://r-spatial.github.io/s2/reference/s2_cell.md)
  : Create S2 Cell vectors
- [`s2_cell_is_valid()`](https://r-spatial.github.io/s2/reference/s2_cell_is_valid.md)
  [`s2_cell_debug_string()`](https://r-spatial.github.io/s2/reference/s2_cell_is_valid.md)
  [`s2_cell_to_lnglat()`](https://r-spatial.github.io/s2/reference/s2_cell_is_valid.md)
  [`s2_cell_center()`](https://r-spatial.github.io/s2/reference/s2_cell_is_valid.md)
  [`s2_cell_boundary()`](https://r-spatial.github.io/s2/reference/s2_cell_is_valid.md)
  [`s2_cell_polygon()`](https://r-spatial.github.io/s2/reference/s2_cell_is_valid.md)
  [`s2_cell_vertex()`](https://r-spatial.github.io/s2/reference/s2_cell_is_valid.md)
  [`s2_cell_level()`](https://r-spatial.github.io/s2/reference/s2_cell_is_valid.md)
  [`s2_cell_is_leaf()`](https://r-spatial.github.io/s2/reference/s2_cell_is_valid.md)
  [`s2_cell_is_face()`](https://r-spatial.github.io/s2/reference/s2_cell_is_valid.md)
  [`s2_cell_area()`](https://r-spatial.github.io/s2/reference/s2_cell_is_valid.md)
  [`s2_cell_area_approx()`](https://r-spatial.github.io/s2/reference/s2_cell_is_valid.md)
  [`s2_cell_parent()`](https://r-spatial.github.io/s2/reference/s2_cell_is_valid.md)
  [`s2_cell_child()`](https://r-spatial.github.io/s2/reference/s2_cell_is_valid.md)
  [`s2_cell_edge_neighbour()`](https://r-spatial.github.io/s2/reference/s2_cell_is_valid.md)
  [`s2_cell_contains()`](https://r-spatial.github.io/s2/reference/s2_cell_is_valid.md)
  [`s2_cell_distance()`](https://r-spatial.github.io/s2/reference/s2_cell_is_valid.md)
  [`s2_cell_max_distance()`](https://r-spatial.github.io/s2/reference/s2_cell_is_valid.md)
  [`s2_cell_may_intersect()`](https://r-spatial.github.io/s2/reference/s2_cell_is_valid.md)
  [`s2_cell_common_ancestor_level()`](https://r-spatial.github.io/s2/reference/s2_cell_is_valid.md)
  [`s2_cell_common_ancestor_level_agg()`](https://r-spatial.github.io/s2/reference/s2_cell_is_valid.md)
  : S2 cell operators

## Utility Functions

- [`s2_earth_radius_meters()`](https://r-spatial.github.io/s2/reference/s2_earth_radius_meters.md)
  : Earth Constants
- [`s2_options()`](https://r-spatial.github.io/s2/reference/s2_options.md)
  [`s2_snap_identity()`](https://r-spatial.github.io/s2/reference/s2_options.md)
  [`s2_snap_level()`](https://r-spatial.github.io/s2/reference/s2_options.md)
  [`s2_snap_precision()`](https://r-spatial.github.io/s2/reference/s2_options.md)
  [`s2_snap_distance()`](https://r-spatial.github.io/s2/reference/s2_options.md)
  : Geography Operation Options
- [`s2_plot()`](https://r-spatial.github.io/s2/reference/s2_plot.md) :
  Plot S2 Geographies

## Example Data

Useful data for testing and demonstrating s2 functions

- [`s2_data_example_wkt`](https://r-spatial.github.io/s2/reference/s2_data_example_wkt.md)
  : Example Geometries
- [`s2_data_tbl_countries`](https://r-spatial.github.io/s2/reference/s2_data_tbl_countries.md)
  [`s2_data_tbl_timezones`](https://r-spatial.github.io/s2/reference/s2_data_tbl_countries.md)
  [`s2_data_tbl_cities`](https://r-spatial.github.io/s2/reference/s2_data_tbl_countries.md)
  [`s2_data_countries()`](https://r-spatial.github.io/s2/reference/s2_data_tbl_countries.md)
  [`s2_data_timezones()`](https://r-spatial.github.io/s2/reference/s2_data_tbl_countries.md)
  [`s2_data_cities()`](https://r-spatial.github.io/s2/reference/s2_data_tbl_countries.md)
  : Low-resolution world boundaries, timezones, and cities

## Low-level Details

Interact with spherical geometry at a low level

- [`wk_handle(`*`<s2_geography>`*`)`](https://r-spatial.github.io/s2/reference/wk_handle.s2_geography.md)
  [`s2_geography_writer()`](https://r-spatial.github.io/s2/reference/wk_handle.s2_geography.md)
  [`wk_writer(`*`<s2_geography>`*`)`](https://r-spatial.github.io/s2/reference/wk_handle.s2_geography.md)
  [`s2_trans_point()`](https://r-spatial.github.io/s2/reference/wk_handle.s2_geography.md)
  [`s2_trans_lnglat()`](https://r-spatial.github.io/s2/reference/wk_handle.s2_geography.md)
  [`s2_projection_plate_carree()`](https://r-spatial.github.io/s2/reference/wk_handle.s2_geography.md)
  [`s2_projection_mercator()`](https://r-spatial.github.io/s2/reference/wk_handle.s2_geography.md)
  [`s2_hemisphere()`](https://r-spatial.github.io/s2/reference/wk_handle.s2_geography.md)
  [`s2_world_plate_carree()`](https://r-spatial.github.io/s2/reference/wk_handle.s2_geography.md)
  [`s2_projection_orthographic()`](https://r-spatial.github.io/s2/reference/wk_handle.s2_geography.md)
  : Low-level wk filters and handlers
