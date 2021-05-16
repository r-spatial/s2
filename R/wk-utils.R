
s2_unprojection_filter <- function(handler, projection = s2_projection_plate_carree(),
                                   tessellate_tol = Inf) {
  wk::new_wk_handler(
    .Call(c_s2_coord_filter_new, handler, projection, TRUE, tessellate_tol),
    subclass = "s2_coord_filter"
  )
}

s2_projection_filter <- function(handler, projection = s2_projection_plate_carree(),
                                 tessellate_tol = Inf) {
  wk::new_wk_handler(
    .Call(c_s2_coord_filter_new, handler, projection, FALSE, tessellate_tol),
    subclass = "s2_coord_filter"
  )
}

s2_projection_plate_carree <- function() {
  .Call(c_s2_projection_plate_carree)
}

s2_projection_mercator <- function() {
  .Call(c_s2_projection_mercator)
}
