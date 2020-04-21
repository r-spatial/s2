
library(tidyverse)

# download S2
source_url <- "https://github.com/google/s2geometry/archive/v0.9.0.zip"
curl::curl_download(source_url, "data-raw/s2-source.tar.gz")
unzip("data-raw/s2-source.tar.gz", exdir = "data-raw")

# make sure the dir exists
s2_dir <- list.files("data-raw", "^s2geometry-[0-9.]+", include.dirs = TRUE, full.names = TRUE)
stopifnot(dir.exists(s2_dir), length(s2_dir) == 1)
src_dir <- file.path(s2_dir, "src/s2")

# headers can be in subdirectories no problem
# it might make more sense to have these in inst/include,
# but I can't get that to work...so they are in src/, with
# the CXXFLAGS=-I../src
headers <- tibble(
  path = list.files(file.path(s2_dir, "src", "s2"), "\\.(h|hpp|cpp)$", full.names = TRUE, recursive = TRUE),
  final_path = str_replace(path, ".*?s2/", "inst/include/s2/")
)

# R doesn't do recursive folders easily
# so instead we replace "/" with "__"
# there aren't any headers in subdirs, so we don't need to
# search + replace in any source files
source_files <- tibble(
  path = list.files(file.path(s2_dir, "src", "s2"), "\\.cc$", full.names = TRUE, recursive = TRUE),
  final_path = str_replace(path, ".*?src/", "src/") %>%
    str_replace_all("/", "__") %>%
    str_replace("^.*?s2__", "src/")
) %>%
  filter(!str_detect(path, "_test\\."))

# clean source dir
current_source_files <- tibble(path = list.files("src", "\\.(h|hpp|cpp|o)$", full.names = TRUE, recursive = TRUE)) %>%
  filter(!str_detect(path, "^src/(libs2-|Rcpp|Makevars)"))

unlink(current_source_files$path)
unlink("inst/include/s2", recursive = TRUE)

# create destination dirs
dest_dirs <- c(
  headers %>% pull(final_path),
  source_files %>% pull(final_path)
) %>%
  dirname() %>%
  unique()
dest_dirs[!dir.exists(dest_dirs)] %>% walk(dir.create, recursive = TRUE)

# copy source files
stopifnot(
  file.copy(headers$path, headers$final_path),
  file.copy(source_files$path, source_files$final_path)
)
