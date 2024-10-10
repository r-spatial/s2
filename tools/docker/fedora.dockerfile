
ARG IMAGE=fedora:latest

FROM ${IMAGE}

RUN dnf install -y R cmake openssl-devel

# Only available on Fedora 39+
RUN dnf install -y abseil-cpp-devel || true

# Make sure we can use all cores to install things
RUN mkdir ~/.R && echo "MAKEFLAGS = -j$(nproc)" > ~/.R/Makevars
RUN R -e 'install.packages(c("wk", "Rcpp", "testthat"), repos = "https://cloud.r-project.org")'

CMD R CMD INSTALL /s2 --preclean && R -e 'testthat::test_local("/s2")'
