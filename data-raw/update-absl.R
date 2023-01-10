
library(tidyverse)
tag <- "20220623.1"

# download Abseil
source_url <- glue::glue("https://github.com/abseil/abseil-cpp/archive/refs/tags/{tag}.zip")
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

unlink("src/absl", recursive = TRUE)

absl_copy(
  glue::glue("data-raw/abseil-cpp-{tag}/absl/container"),
  "src/absl/container"
)

absl_copy(
  glue::glue("data-raw/abseil-cpp-{tag}/absl/base"),
  "src/absl/base"
)

absl_copy(
  glue::glue("data-raw/abseil-cpp-{tag}/absl/meta"),
  "src/absl/meta"
)

absl_copy(
  glue::glue("data-raw/abseil-cpp-{tag}/absl/synchronization"),
  "src/absl/synchronization"
)

absl_copy(
  glue::glue("data-raw/abseil-cpp-{tag}/absl/time"),
  "src/absl/time"
)

absl_copy(
  glue::glue("data-raw/abseil-cpp-{tag}/absl/strings"),
  "src/absl/strings"
)

absl_copy(
  glue::glue("data-raw/abseil-cpp-{tag}/absl/utility"),
  "src/absl/utility"
)

absl_copy(
  glue::glue("data-raw/abseil-cpp-{tag}/absl/debugging"),
  "src/absl/debugging"
)

absl_copy(
  glue::glue("data-raw/abseil-cpp-{tag}/absl/memory"),
  "src/absl/memory"
)

absl_copy(
  glue::glue("data-raw/abseil-cpp-{tag}/absl/types"),
  "src/absl/types"
)

absl_copy(
  glue::glue("data-raw/abseil-cpp-{tag}/absl/numeric"),
  "src/absl/numeric"
)

absl_copy(
  glue::glue("data-raw/abseil-cpp-{tag}/absl/algorithm"),
  "src/absl/algorithm"
)

absl_copy(
  glue::glue("data-raw/abseil-cpp-{tag}/absl/functional"),
  "src/absl/functional"
)

absl_copy(
  glue::glue("data-raw/abseil-cpp-{tag}/absl/profiling"),
  "src/absl/profiling"
)

absl_objects <- list.files("src/absl", ".cc$", recursive = TRUE) %>%
  file.path("absl", .) %>%
  str_subset("\\.cc$") %>%
  str_replace("\\.cc$", ".o") %>%
  paste0(collapse = " \\\n    ") %>%
  paste0("ABSL_LIBS = ", .)

clipr::write_clip(absl_objects)
usethis::edit_file("src/Makevars.win")
usethis::edit_file("src/Makevars.in")

# Edits needed to make CMD check happy

# Pragmas
fix_pragmas <- function(f) {
  content <- readr::read_file(f)
  content <- stringr::str_replace_all(content, "\n#pragma", "\n// #pragma")
  readr::write_file(content, f)
}

fix_pragmas("src/absl/base/internal/invoke.h")
fix_pragmas("src/absl/container/inlined_vector.h")
fix_pragmas("src/absl/container/internal/inlined_vector.h")
fix_pragmas("src/absl/functional/internal/any_invocable.h")
fix_pragmas("src/absl/types/internal/optional.h")
fix_pragmas("src/absl/container/internal/counting_allocator.h")

# Aborts
fix_aborts <- function(f) {
  content <- readr::read_file(f)
  content <- stringr::str_replace_all(content, fixed("abort()"), "throw std::runtime_error(\"abort()\")")
  readr::write_file(content, f)
}

fix_aborts("src/absl/base/internal/raw_logging.cc")
fix_aborts("src/absl/base/internal/sysinfo.cc")
fix_aborts("src/absl/debugging/symbolize_elf.inc")

# Manual updates

# The symbolizer implementation causes some trouble. We don't use this feature here
# and there seems to be a way to turn it off completely. Do this.
usethis::edit_file("src/absl/debugging/symbolize.cc")

# On Windows, R.h defines a macro 'Free', which we have to undefine
usethis::edit_file("src/absl/base/internal/low_level_alloc.h")

# On Windows with rtools35 (i.e., very old GCC with incomplete C++11), a reference
# to std::get_time() causes compilation error. We don't need strptime here, so just
# return nullptr in this function.
usethis::edit_file("src/absl/time/internal/cctz/src/time_zone_format.cc")

# Windows builds have some additional issues with format strings. These are all within
# absl logger functions...just remove the definition of ABSL_RAW_LOG(...).
usethis::edit_file("src/absl/base/internal/raw_logging.h")

# Fix a workaround for older gcc that causes a check warning. The bug that the
# workaround is addressing only applies to old gcc, so only use that bit of code
# for old gcc
usethis::edit_file("src/absl/container/internal/raw_hash_set.h")

# CRAN compiles with -Wpedantic, so we can't use the __int128 intrinsic type
# undefine ABSL_HAVE_INTRINSIC_INT128 here:
usethis::edit_file("src/absl/base/config.h")
