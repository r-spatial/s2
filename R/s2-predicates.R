#' S2 Geography Predicates
#'
#' @inheritParams s2_is_collection
#' @param lng1,lat1,lng2,lat2 A latitude/longitude range
#' @param distance A distance in meters on the surface of the earth
#' @param detail The number of points with which to approximate
#'   non-geodesic edges.
#' @param model integer indicating the geometry model used; see Details.
#' @param ... arguments passed on
#'
#' @details
#' The geometry model indicates whether a geometry includes its boundaries.
#' Boundaries of line geometries are its end points.
#' OPEN geometries do not contain their boundary (model = 0); CLOSED
#' geometries (model = 2) contain their boundary; HALF-CLOSED geometries 
#' contain, like, half of their boundaries, such that when two polygons
#' do not overlap or two lines do not cross, no point exist that belong to 
#' more than one of the geometries. (This latter form, half-closed, is 
#' not present n the OpenGIS "simple feature access" (SFA) standard, or DE9-IM on 
#' which that is based). A value of -1 does not set the model, leaving the
#' S2 default (HALF-CLOSED). The default values for \code{s2_contains} (0)
#' and covers/coveredby (2) correspond to the SFA standard specification 
#' of these operators.
#' 
#' @export
#'
#' @seealso
#' BigQuery's geography function reference:
#'
#' - [ST_CONTAINS](https://cloud.google.com/bigquery/docs/reference/standard-sql/geography_functions#st_contains)
#' - [ST_COVEREDBY](https://cloud.google.com/bigquery/docs/reference/standard-sql/geography_functions#st_coveredby)
#' - [ST_COVERS](https://cloud.google.com/bigquery/docs/reference/standard-sql/geography_functions#st_covers)
#' - [ST_DISJOINT](https://cloud.google.com/bigquery/docs/reference/standard-sql/geography_functions#st_disjoint)
#' - [ST_EQUALS](https://cloud.google.com/bigquery/docs/reference/standard-sql/geography_functions#st_equals)
#' - [ST_INTERSECTS](https://cloud.google.com/bigquery/docs/reference/standard-sql/geography_functions#st_intersects)
#' - [ST_INTERSECTSBOX](https://cloud.google.com/bigquery/docs/reference/standard-sql/geography_functions#st_intersectsbox)
#' - [ST_TOUCHES](https://cloud.google.com/bigquery/docs/reference/standard-sql/geography_functions#st_touches)
#' - [ST_WITHIN](https://cloud.google.com/bigquery/docs/reference/standard-sql/geography_functions#st_within)
#' - [ST_DWITHIN](https://cloud.google.com/bigquery/docs/reference/standard-sql/geography_functions#st_dwithin)
#'
s2_contains <- function(x, y, model = 0) {
  recycled <- recycle_common(as_s2_geography(x), as_s2_geography(y))
  cpp_s2_contains(recycled[[1]], recycled[[2]], model = model)
}

#' @rdname s2_contains
#' @export
s2_covered_by <- function(x, y, model = 2) {
  s2_covers(y, x, model = model)
}

#' @rdname s2_contains
#' @export
s2_covers <- function(x, y, model = 2) {
  recycled <- recycle_common(as_s2_geography(x), as_s2_geography(y))
  cpp_s2_contains(recycled[[1]], recycled[[2]], model = model)
}

#' @rdname s2_contains
#' @export
s2_disjoint <- function(x, y, ...) {
  !s2_intersects(x, y, ...)
}

#' @rdname s2_contains
#' @export
s2_equals <- function(x, y, model = -1) {
  recycled <- recycle_common(as_s2_geography(x), as_s2_geography(y))
  cpp_s2_equals(recycled[[1]], recycled[[2]], model)
}

sort_out_model = function(x) {
  switch(x,
    OPEN = 0,
    SEMI_OPEN = 1,
    CLOSED = 2,
    -1)
}

#' @rdname s2_contains
#' @param model integer or character; specify polygon and polyline model 
#' as "OPEN" (or 0), "SEMI_OPEN" (or 1), or "CLOSED" (or 2)
#' @export
s2_intersects <- function(x, y, model = -1) {
  if (!is.numeric(model))
    model = sort_out_model(as.character(model))
  recycled <- recycle_common(as_s2_geography(x), as_s2_geography(y))
  cpp_s2_intersects(recycled[[1]], recycled[[2]], model)
}

#' @rdname s2_contains
#' @export
s2_intersects_box <- function(x, lng1, lat1, lng2, lat2, detail = 1000, model = -1) {
  recycled <- recycle_common(as_s2_geography(x), lng1, lat1, lng2, lat2, detail)
  cpp_s2_intersects_box(
    recycled[[1]],
    recycled[[2]], recycled[[3]],
    recycled[[4]], recycled[[5]],
    detail = recycled[[6]],
	model
  )
}

#' @rdname s2_contains
#' @export
s2_touches <- function(x, y) {
  stop("Not implemented")
}

#' @rdname s2_contains
#' @export
s2_within <- function(x, y, ...) {
  s2_contains(y, x, ...)
}

#' @rdname s2_contains
#' @export
s2_dwithin <- function(x, y, distance, radius = s2_earth_radius_meters()) {
  recycled <- recycle_common(as_s2_geography(x), as_s2_geography(y), distance / radius)
  cpp_s2_dwithin(recycled[[1]], recycled[[2]], recycled[[3]])
}
