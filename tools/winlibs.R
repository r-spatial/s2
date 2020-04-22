# Build against openssl libraries that were compiled with the Rtools gcc toolchain.
if(!file.exists("../windows/openssl-1.1.1/include/openssl/ssl.h")){
  if(getRversion() < "3.3.0") setInternet2()
  download.file("https://github.com/rwinlib/openssl/archive/v1.1.1.zip", "lib.zip", quiet = TRUE)
  dir.create("../windows", showWarnings = FALSE)
  unzip("lib.zip", exdir = "../windows")
  unlink("lib.zip")
}
