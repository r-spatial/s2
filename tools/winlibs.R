
# This script is used to support R 4.0, 4.1, and 4.2 on Windows, which do not have
# OpenSSL available via pkg-config. It places the headers in windows/openssl/include
# and the libs for this R version/arch in windows/openssl/lib to simplify the
# configure/makevars setup.

openssl_version <- "1.1.1k"
if(file.exists("windows/openssl/include/openssl/ssl.h")) {
  cat("Using previously downloaded rwinlibs openssl")
} else {
  cat(sprintf("Downloading OpenSSL %s from rwinlibs", openssl_version))

  download.file(sprintf("https://github.com/rwinlib/openssl/archive/v%s.zip", openssl_version),
                "lib.zip", quiet = TRUE)
  dir.create("../windows", showWarnings = FALSE)
  unzip("lib.zip", exdir = "windows")

  file.rename("windows/openssl-1.1.1k", "windows/openssl")
  crt <- if (packageVersion("base") >= "4.2.0") "-ucrt" else ""
  libs <- list.files(
    sprintf("windows/openssl/lib/%s%s", .Platform$r_arch, crt),
    full.names = TRUE
  )

  file.copy(libs, "windows/openssl/lib")
  unlink("lib.zip")
}
