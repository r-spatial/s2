
library(tidyverse)
tag <- "20230802.1"

# download Abseil
source_url <- glue::glue("https://github.com/abseil/abseil-cpp/archive/refs/tags/{tag}.zip")
curl::curl_download(source_url, "data-raw/abseil-cpp-source.zip")
unzip("data-raw/abseil-cpp-source.zip", exdir = "tools/vendor")
file.rename(glue::glue("tools/vendor/abseil-cpp-{tag}"), "tools/vendor/abseil-cpp")

# prune unused components
unlink("tools/vendor/abseil-cpp/.github", recursive = TRUE)
unlink("tools/vendor/abseil-cpp/ci", recursive = TRUE)
unlink(list.files("tools/vendor/abseil-cpp", "\\.py$", full.names = TRUE))
unlink(
  list.files(
    "tools/vendor/abseil-cpp/absl", "_test.cc$",
    full.names = TRUE,
    recursive = TRUE
  )
)
unlink(
  list.files(
    "tools/vendor/abseil-cpp/absl", "_benchmark.cc$",
    full.names = TRUE,
    recursive = TRUE
  )
)
