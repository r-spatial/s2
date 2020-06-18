
#' S2 Geography Transformations
#'
#' @inheritParams s2_is_collection
#' @param na.rm For aggregate calculations use `na.rm = TRUE`
#'   to drop missing values.
#' @param model integer; specifies boundary model. See [s2_model_default()].
#' @param snap_level integer; if positive, specifies the snap level. See [s2_snap_default()].
#'
#' @export
#'
#' @seealso
#' BigQuery's geography function reference:
#'
#' - [ST_BOUNDARY](https://cloud.google.com/bigquery/docs/reference/standard-sql/geography_functions#st_boundary)
#' - [ST_CENTROID](https://cloud.google.com/bigquery/docs/reference/standard-sql/geography_functions#st_centroid)
#' - [ST_CLOSESTPOINT](https://cloud.google.com/bigquery/docs/reference/standard-sql/geography_functions#st_closestpoint)
#' - [ST_DIFFERENCE](https://cloud.google.com/bigquery/docs/reference/standard-sql/geography_functions#st_difference)
#' - [ST_INTERSECTION](https://cloud.google.com/bigquery/docs/reference/standard-sql/geography_functions#st_intersection)
#' - [ST_UNION](https://cloud.google.com/bigquery/docs/reference/standard-sql/geography_functions#st_union)
#' - [ST_SNAPTOGRID](https://cloud.google.com/bigquery/docs/reference/standard-sql/geography_functions#st_snaptogrid)
#' - [ST_UNION_AGG](https://cloud.google.com/bigquery/docs/reference/standard-sql/geography_functions#st_union_agg)
#' - [ST_CENTROID_AGG](https://cloud.google.com/bigquery/docs/reference/standard-sql/geography_functions#s2_centroid_agg)
#'
s2_boundary <- function(x) {
  new_s2_xptr(cpp_s2_boundary(as_s2_geography(x)), "s2_geography")
}

#' @rdname s2_boundary
#' @export
s2_centroid <- function(x) {
  new_s2_xptr(cpp_s2_centroid(as_s2_geography(x)), "s2_geography")
}

#' @rdname s2_boundary
#' @export
s2_closest_point <- function(x, y) {
  recycled <- recycle_common(as_s2_geography(x), as_s2_geography(y))
  new_s2_xptr(cpp_s2_closest_point(recycled[[1]], recycled[[2]]), "s2_geography")
}

#' @rdname s2_boundary
#' @export
s2_nearest_feature <- function(x, y) {
  recycled <- recycle_common(as_s2_geography(y), as_s2_geography(x)) # REVERSING x and y HERE!!
  unlist(cpp_s2_nearest_feature(recycled[[1]], recycled[[2]]))
}

#' @rdname s2_boundary
#' @export
s2_difference <- function(x, y, model = s2_model_default(), snap_level = s2_snap_default()) {
  recycled <- recycle_common(as_s2_geography(x), as_s2_geography(y))
  new_s2_xptr(cpp_s2_difference(recycled[[1]], recycled[[2]], model, snap_level), "s2_geography")
}

#' @rdname s2_boundary
#' @export
s2_sym_difference <- function(x, y, model = s2_model_default(), snap_level = s2_snap_default()) {
  recycled <- recycle_common(as_s2_geography(x), as_s2_geography(y))
  new_s2_xptr(cpp_s2_sym_difference(recycled[[1]], recycled[[2]], model, snap_level), "s2_geography")
}

#' @rdname s2_boundary
#' @export
s2_intersection <- function(x, y, model = s2_model_default(), snap_level = s2_snap_default()) {
  recycled <- recycle_common(as_s2_geography(x), as_s2_geography(y))
  new_s2_xptr(cpp_s2_intersection(recycled[[1]], recycled[[2]], model, snap_level), "s2_geography")
}

#' @rdname s2_boundary
#' @export
s2_union <- function(x, y = NULL, model = s2_model_default(), snap_level = s2_snap_default()) {
  if (is.null(y)) {
    y <- as_s2_geography("POINT EMPTY")
  }

  recycled <- recycle_common(as_s2_geography(x), as_s2_geography(y))
  new_s2_xptr(cpp_s2_union(recycled[[1]], recycled[[2]], model, snap_level), "s2_geography")
}

#' @rdname s2_boundary
#' @export
s2_snap_to_grid <- function(x) {
  stop("Not implemented")
}

#' @rdname s2_boundary
#' @export
s2_union_agg <- function(x, model = s2_model_default(), snap_level = s2_snap_default(), na.rm = FALSE) {
  new_s2_xptr(cpp_s2_union_agg(as_s2_geography(x), model, snap_level, na.rm), "s2_geography")
}

#' @rdname s2_boundary
#' @export
s2_centroid_agg <- function(x, na.rm = FALSE) {
  new_s2_xptr(cpp_s2_centroid_agg(as_s2_geography(x), naRm = na.rm), "s2_geography")
}
