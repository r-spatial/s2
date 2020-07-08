
vec_proxy.s2_geography <- function(x, ...) {
  unclass(x)
}

vec_restore.s2_geography <- function(x, ...) {
  new_s2_xptr(x, "s2_geography")
}

vec_ptype_abbr.s2_geography <- function(x, ...) {
  "s2_geography"
}

vec_proxy.s2_point <- function(x, ...) {
  unclass(x)
}

vec_restore.s2_point <- function(x, ...) {
  new_s2_xptr(x, "s2_point")
}

vec_ptype_abbr.s2_point <- function(x, ...) {
  "s2_point"
}

vec_proxy.s2_lnglat <- function(x, ...) {
  unclass(x)
}

vec_restore.s2_lnglat <- function(x, ...) {
  new_s2_xptr(x, "s2_lnglat")
}

vec_ptype_abbr.s2_lnglat <- function(x, ...) {
  "s2_lnglat"
}
