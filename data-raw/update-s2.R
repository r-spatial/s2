
library(tidyverse)

tag <- "0.10.0"

# download + vendor S2
source_url <- glue::glue("https://github.com/google/s2geometry/archive/v{tag}.zip")
curl::curl_download(source_url, "data-raw/s2geometry-source.zip")
unzip("data-raw/s2geometry-source.zip", exdir = "tools/vendor")
unlink("data-raw/s2geometry-source.zip")
file.rename(glue::glue("tools/vendor/s2geometry-{tag}"), "tools/vendor/s2-geometry")

# prune unused components
unlink("tools/vendor/s2-geometry/doc", recursive = TRUE)
unlink("tools/vendor/s2-geometry/src/python", recursive = TRUE)
unlink(list.files("tools/vendor/s2-geometry/src", "_test.cc$", full.names = TRUE, recursive = TRUE))
