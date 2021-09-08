
library(tidyverse)

# download Abseil
source_url <- "https://github.com/abseil/abseil-cpp/archive/refs/tags/20210324.2.zip"
curl::curl_download(source_url, "data-raw/abseil-cpp-source.zip")
unzip("data-raw/abseil-cpp-source.zip", exdir = "data-raw")


absl_copy <- function(src, dst) {
  src_files <- list.files(src, "\\.(cc|h|inc)$", recursive = TRUE) %>%
    str_subset("_test(ing)?\\.(cc|h)$", negate = TRUE) %>%
    str_subset("test_", negate = TRUE) %>%
    str_subset("_benchmark", negate = TRUE)

  dst_files <- file.path(dst, src_files)
  dst_dirs <- unique(dirname(dst_files))
  for (d in sort(dst_dirs)) {
    if (!dir.exists(d)) dir.create(d, recursive = TRUE)
  }

  stopifnot(all(file.copy(file.path(src, src_files), dst_files)))
}

absl_copy(
  "data-raw/abseil-cpp-20210324.2/absl/container",
  "src/absl/container"
)

absl_copy(
  "data-raw/abseil-cpp-20210324.2/absl/base",
  "src/absl/base"
)

absl_copy(
  "data-raw/abseil-cpp-20210324.2/absl/meta",
  "src/absl/meta"
)

absl_copy(
  "data-raw/abseil-cpp-20210324.2/absl/synchronization",
  "src/absl/synchronization"
)

absl_copy(
  "data-raw/abseil-cpp-20210324.2/absl/time",
  "src/absl/time"
)

absl_copy(
  "data-raw/abseil-cpp-20210324.2/absl/strings",
  "src/absl/strings"
)

absl_copy(
  "data-raw/abseil-cpp-20210324.2/absl/utility",
  "src/absl/utility"
)

absl_copy(
  "data-raw/abseil-cpp-20210324.2/absl/debugging",
  "src/absl/debugging"
)

absl_copy(
  "data-raw/abseil-cpp-20210324.2/absl/memory",
  "src/absl/memory"
)

absl_copy(
  "data-raw/abseil-cpp-20210324.2/absl/types",
  "src/absl/types"
)

absl_copy(
  "data-raw/abseil-cpp-20210324.2/absl/numeric",
  "src/absl/numeric"
)

absl_copy(
  "data-raw/abseil-cpp-20210324.2/absl/algorithm",
  "src/absl/algorithm"
)

absl_copy(
  "data-raw/abseil-cpp-20210324.2/absl/functional",
  "src/absl/functional"
)

absl_objects <- list.files("src/absl", ".cc$", recursive = TRUE) %>%
  file.path("absl", .) %>%
  str_subset("\\.cc$") %>%
  str_replace("\\.cc$", ".o") %>%
  paste0(collapse = " \\\n    ") %>%
  paste0("ABSL_LIBS = ", .)

clipr::write_clip(absl_objects)
