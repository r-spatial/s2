
#' Create and Format Geography Vectors
#'
#' These functions create and export [geography vectors][as_s2_geography].
#' Unlike the BigQuery geography constructors, these functions do not sanitize
#' invalid or redundant input using [s2_union()]. Note that when creating polygons
#' using [s2_make_polygon()], rings can be open or closed.
#'
#' @inheritParams s2_is_collection
#' @inheritParams as_s2_geography
#' @param precision The number of significant digits to export when
#'   writing well-known text. If `trim = FALSE`, the number of
#'   digits after the decimal place.
#' @param trim Should trailing zeroes be included after the decimal place?
#' @param endian The endian-ness of the well-known binary. See [wk::wkb_translate_wkb()].
#' @param longitude,latitude Vectors of latitude and longitude
#' @param wkt_string Well-known text
#' @param wkb_bytes A `list()` of `raw()`
#' @param planar Use `TRUE` to force planar edges in import or export.
#' @param tessellate_tol_m The maximum number of meters to that a point must
#'   be moved to satisfy the planar edge constraint.
#' @param feature_id,ring_id Vectors for which a change in
#'   sequential values indicates a new feature or ring. Use [factor()]
#'   to convert from a character vector.
#'
#' @export
#'
#' @seealso
#' See [as_s2_geography()] for other ways to construct geography vectors.
#'
#' BigQuery's geography function reference:
#'
#' - [ST_GEOGPOINT](https://cloud.google.com/bigquery/docs/reference/standard-sql/geography_functions#st_geogpoint)
#' - [ST_MAKELINE](https://cloud.google.com/bigquery/docs/reference/standard-sql/geography_functions#st_makeline)
#' - [ST_MAKEPOLYGON](https://cloud.google.com/bigquery/docs/reference/standard-sql/geography_functions#st_makepolygon)
#' - [ST_GEOGFROMTEXT](https://cloud.google.com/bigquery/docs/reference/standard-sql/geography_functions#st_geogfromtext)
#' - [ST_GEOGFROMWKB](https://cloud.google.com/bigquery/docs/reference/standard-sql/geography_functions#st_geogfromwkb)
#' - [ST_ASTEXT](https://cloud.google.com/bigquery/docs/reference/standard-sql/geography_functions#st_astext)
#' - [ST_ASBINARY](https://cloud.google.com/bigquery/docs/reference/standard-sql/geography_functions#st_asbinary)
#'
#' @examples
#' # create point geographies using coordinate values:
#' s2_geog_point(-64, 45)
#'
#' # create line geographies using coordinate values:
#' s2_make_line(c(-64, 8), c(45, 71))
#'
#' # optionally, separate features using feature_id:
#' s2_make_line(
#'   c(-64, 8, -27, -27), c(45, 71, 0, 45),
#'   feature_id = c(1, 1, 2, 2)
#' )
#'
#' # create polygon geographies using coordinate values:
#' # (rings can be open or closed)
#' s2_make_polygon(c(-45, 8, 0), c(64, 71, 90))
#'
#' # optionally, separate rings and/or features using
#' # ring_id and/or feature_id
#' s2_make_polygon(
#'   c(20, 10, 10, 30, 45, 30, 20, 20, 40, 20, 45),
#'   c(35, 30, 10, 5, 20, 20, 15, 25, 40, 45, 30),
#'   feature_id = c(rep(1, 8), rep(2, 3)),
#'   ring_id = c(1, 1, 1, 1, 1, 2, 2, 2, 1, 1, 1)
#' )
#'
#' # import and export well-known text
#' (geog <- s2_geog_from_text("POINT (-64 45)"))
#' s2_as_text(geog)
#'
#' # import and export well-known binary
#' (geog <- s2_geog_from_wkb(wk::as_wkb("POINT (-64 45)")))
#' s2_as_binary(geog)
#'
#' # import geometry from planar space
#' s2_geog_from_text(
#'    "POLYGON ((0 0, 1 0, 0 1, 0 0))",
#'    planar = TRUE,
#'    tessellate_tol_m = 1
#' )
#'
#' # export geographies into planar space
#' geog <- s2_make_polygon(c(179, -179, 179), c(10, 10, 11))
#' s2_as_text(geog, planar = TRUE)
#'
#' # polygons containing a pole need an extra step
#' geog <- s2_data_countries("Antarctica")
#' geom <- s2_as_text(
#'   s2_intersection(geog, s2_world_plate_carree()),
#'   planar = TRUE
#' )
#'
s2_geog_point <- function(longitude, latitude) {
  wk::wk_handle(wk::xy(longitude, latitude), s2_geography_writer())
}

#' @rdname s2_geog_point
#' @export
s2_make_line <- function(longitude, latitude, feature_id = 1L) {
  wk::wk_handle(
    wk::xy(longitude, latitude),
    wk::wk_linestring_filter(
      s2_geography_writer(),
      feature_id = as.integer(feature_id)
    )
  )
}

#' @rdname s2_geog_point
#' @export
s2_make_polygon <- function(longitude, latitude, feature_id = 1L, ring_id = 1L,
                            oriented = FALSE, check = TRUE) {
  wk::wk_handle(
    wk::xy(longitude, latitude),
    wk::wk_polygon_filter(
      s2_geography_writer(oriented = oriented, check = check),
      feature_id = as.integer(feature_id),
      ring_id = as.integer(ring_id)
    )
  )
}

#' @rdname s2_geog_point
#' @export
s2_geog_from_text <- function(wkt_string, oriented = FALSE, check = TRUE,
                              planar = FALSE,
                              tessellate_tol_m = s2_tessellate_tol_default()) {
  attributes(wkt_string) <- NULL
  wkt <- wk::new_wk_wkt(wkt_string, geodesic = TRUE)
  wk::validate_wk_wkt(wkt)

  wk::wk_handle(
    wkt,
    s2_geography_writer(
      oriented = oriented,
      check = check,
      tessellate_tol = if (planar) {
        tessellate_tol_m / s2_earth_radius_meters()
      } else {
        Inf
      }
    )
  )
}

#' @rdname s2_geog_point
#' @export
s2_geog_from_wkb <- function(wkb_bytes, oriented = FALSE, check = TRUE,
                             planar = FALSE,
                             tessellate_tol_m = s2_tessellate_tol_default()) {
  attributes(wkb_bytes) <- NULL
  wkb <- wk::new_wk_wkb(wkb_bytes)
  wk::validate_wk_wkb(wkb)
  wk::wk_handle(
    wkb,
    s2_geography_writer(
      oriented = oriented,
      check = check,
      tessellate_tol = if (planar) {
        tessellate_tol_m / s2_earth_radius_meters()
      } else {
        Inf
      }
    )
  )
}

#' @rdname s2_geog_point
#' @export
s2_as_text <- function(x, precision = 16, trim = TRUE,
                       planar = FALSE,
                       tessellate_tol_m = s2_tessellate_tol_default()) {
  wkt <- wk::wk_handle(
    as_s2_geography(x),
    wk::wkt_writer(precision = precision, trim = trim),
    s2_tessellate_tol = if (planar) {
      tessellate_tol_m / s2_earth_radius_meters()
    } else {
      Inf
    }
  )

  attributes(wkt) <- NULL
  wkt
}

#' @rdname s2_geog_point
#' @export
s2_as_binary <- function(x, endian = wk::wk_platform_endian(),
                         planar = FALSE,
                         tessellate_tol_m = s2_tessellate_tol_default()) {
  structure(
    wk::wk_handle(
      as_s2_geography(x),
      wk::wkb_writer(endian = endian),
      s2_tessellate_tol = if (planar) {
        tessellate_tol_m / s2_earth_radius_meters()
      } else {
        Inf
      }
    ),
    class = "blob"
  )
}

#' @rdname s2_geog_point
#' @export
s2_tessellate_tol_default <- function() {
  100
}
