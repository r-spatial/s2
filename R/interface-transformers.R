
#' S2 Geography Transformations
#'
#' @inheritParams s2_iscollection
#' @param na.rm For aggregate calculations use `na.rm = TRUE`
#'   to drop missing values.
#' @param model integer; specifies boundary model; see \link{s2_contains}
#' @param snap_level integer; if positive, specifies the snap level; see \link{s2_set_snaplevel}
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
  new_s2xptr(libs2_cpp_s2_boundary(s2geography(x)), "s2geography")
}

#' @rdname s2_boundary
#' @export
s2_centroid <- function(x) {
  new_s2xptr(libs2_cpp_s2_centroid(s2geography(x)), "s2geography")
}

#' @rdname s2_boundary
#' @export
s2_closestpoint <- function(x, y) {
  recycled <- recycle_common(s2geography(x), s2geography(y))
  new_s2xptr(libs2_cpp_s2_closestpoint(recycled[[1]], recycled[[2]]), "s2geography")
}

#' @rdname s2_boundary
#' @export
s2_difference <- function(x, y, model = -1L, snap_level = -1L) {
  on.exit(s2_set_snaplevel(s2_set_snaplevel(snap_level)))
  recycled <- recycle_common(s2geography(x), s2geography(y))
  new_s2xptr(libs2_cpp_s2_difference(recycled[[1]], recycled[[2]], model), "s2geography")
}

#' @rdname s2_boundary
#' @export
s2_symdifference <- function(x, y, model = -1L, snap_level = -1L) {
  on.exit(s2_set_snaplevel(s2_set_snaplevel(snap_level)))
  recycled <- recycle_common(s2geography(x), s2geography(y))
  new_s2xptr(libs2_cpp_s2_symdifference(recycled[[1]], recycled[[2]], model), "s2geography")
}

#' @rdname s2_boundary
#' @export
s2_intersection <- function(x, y, model = -1L, snap_level = -1L) {
  on.exit(s2_set_snaplevel(s2_set_snaplevel(snap_level)))
  recycled <- recycle_common(s2geography(x), s2geography(y))
  new_s2xptr(libs2_cpp_s2_intersection(recycled[[1]], recycled[[2]], model), "s2geography")
}

#' @rdname s2_boundary
#' @export
s2_union <- function(x, y = NULL, model = -1L, snap_level = -1L) {
  on.exit(s2_set_snaplevel(s2_set_snaplevel(snap_level)))
  if (is.null(y)) {
    y <- s2geography("POINT EMPTY")
  }

  recycled <- recycle_common(s2geography(x), s2geography(y))
  new_s2xptr(libs2_cpp_s2_union(recycled[[1]], recycled[[2]], model), "s2geography")
}

#' @rdname s2_boundary
#' @export
s2_snaptogrid <- function(x) {
  stop("Not implemented")
}

#' @rdname s2_boundary
#' @export
s2_union_agg <- function(x, na.rm = FALSE) {
  new_s2xptr(libs2_cpp_s2_union_agg(s2geography(x), na.rm), "s2geography")
}

#' @rdname s2_boundary
#' @export
s2_centroid_agg <- function(x, na.rm = FALSE) {
  new_s2xptr(libs2_cpp_s2_centroid_agg(s2geography(x), na.rm), "s2geography")
}

