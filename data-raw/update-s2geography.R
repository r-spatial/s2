
library(tidyverse)

tag <- "764682d60f757383e5853a45a12404c84f3e5961"

# download + vendor S2
source_url <- glue::glue("https://github.com/paleolimbot/s2geography/archive/{tag}.zip")
curl::curl_download(source_url, "data-raw/s2geography-source.zip")
unzip("data-raw/s2geography-source.zip", exdir = "tools/vendor")
unlink("data-raw/s2geography-source.zip")
file.rename(glue::glue("tools/vendor/s2geography-{tag}"), "tools/vendor/s2geography")
