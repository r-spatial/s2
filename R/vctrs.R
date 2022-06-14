
vec_proxy.s2_geography <- function(x, ...) {
  unclass(x)
}

vec_restore.s2_geography <- function(x, ...) {
  new_s2_geography(x)
}

vec_ptype_abbr.s2_geography <- function(x, ...) {
  "s2_geography"
}

vec_proxy.s2_cell <- function(x, ...) {
  unclass(x)
}

vec_restore.s2_cell <- function(x, ...) {
  new_s2_cell(x)
}

vec_ptype_abbr.s2_cell <- function(x, ...) {
  "s2cell"
}

vec_proxy.s2_cell_union <- function(x, ...) {
  unclass(x)
}

vec_restore.s2_cell_union <- function(x, ...) {
  new_s2_cell_union(x)
}

vec_ptype_abbr.s2_cell_union <- function(x, ...) {
  "s2cellunion"
}
