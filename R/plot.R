
#' @export
plot.s2_geography <- function(x, ..., asp = 1, xlab = "", ylab = "",
                              rule = "evenodd", add = FALSE,
                              plot_hemisphere = FALSE,
                              centre = s2_centroid_agg(x, na.rm = TRUE)) {
  # get the projected bbox before plotting
  centre <- as_s2_lnglat(centre)
  projection <- s2_projection_orthographic(centre)
  hemisphere_bounds_poly <- cap_to_polygon(centre, (pi / 2) - 1e-5)

  if (plot_hemisphere) {
    bbox_projected <- wk::rct(-1, -1, 1, 1)
  } else if (!add) {
    bbox_projected <- wk::wk_handle(
      x,
      wk::wk_bbox_handler(),
      s2_projection = projection,
      s2_tessellate_tol = s2_tessellate_tol_default()
    )
  }

  wk::wk_plot(
    wk::xy(),
    bbox = bbox_projected,
    asp = asp, xlab = xlab, ylab = ylab,
    add = add
  )

  if (plot_hemisphere) {
    wk::wk_plot(wk::crc(0, 0, 1), add = TRUE)
  }

  # estimate resolution. In user coords, this can be though of in radians
  # (at the centre of the plot)
  usr <- graphics::par("usr")
  usr_x <- usr[1:2]
  usr_y <- usr[3:4]
  device_x <- graphics::grconvertX(usr_x, to = "device")
  device_y <- graphics::grconvertY(usr_y, to = "device")

  scale_x <- diff(device_x) / diff(usr_x)
  scale_y <- diff(device_y) / diff(usr_y)
  scale <- min(abs(scale_x), abs(scale_y))
  resolution_usr_rad <- 0.25 / scale

  # limit output to dimensions in input
  dimensions_in_input <- unique(s2_dimension(x))

  x_hemisphere <- s2_intersection(
    x,
    hemisphere_bounds_poly,
    options = s2_options(
      snap = s2_snap_distance(resolution_usr_rad),
      snap_radius = resolution_usr_rad,
      simplify_edge_chains = TRUE,
      dimensions = c("point", "polyline", "polygon")[dimensions_in_input + 1]
    )
  )

  x_hemisphere[s2_is_empty(x_hemisphere)] <- as_s2_geography("POINT EMPTY")

  x_hemisphere_planar <- wk::wk_handle(
    x_hemisphere,
    wk::wkb_writer(),
    s2_projection = projection,
    s2_tessellate_tol = max(0.002, resolution_usr_rad * 4)
  )

  wk::wk_plot(
    x_hemisphere_planar,
    ...,
    rule = rule,
    add = TRUE
  )

  invisible(x)
}

cap_to_polygon <- function(centre = s2_lnglat(0, 0), radius_rad) {
  centre <- as_s2_lnglat(centre)
  rad_proj <- sin(radius_rad)
  points <- wk::xy(
    c(0, rad_proj, 0, -rad_proj, 0),
    c(rad_proj, 0, -rad_proj, 0, rad_proj)
  )
  points_s2 <- wk::wk_handle(
    points,
    s2_geography_writer(
      projection = s2_projection_orthographic(centre)
    )
  )
  s2_make_polygon(s2_x(points_s2), s2_y(points_s2))
}
