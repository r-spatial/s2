skip_if_serialization_unsupported <- function(...) {
  skip_if(getRversion() < "4.3.0")
}

skip_if_serialization_supported <- function(...) {
  skip_if(getRversion() >= "4.3.0")
}
