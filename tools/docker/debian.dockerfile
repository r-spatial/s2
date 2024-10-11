
ARG IMAGE=debian:testing

FROM ${IMAGE}

RUN apt-get update && DEBIAN_FRONTEND=noninteractive apt-get install -y \
    r-base cmake libcurl4-openssl-dev libssl-dev

# Not all versions of ubuntu/debian have libabsl-dev
RUN apt-get install -y libabsl-dev || true

# Make sure we can use all cores to install things
RUN mkdir ~/.R && echo "MAKEFLAGS = -j$(nproc)" > ~/.R/Makevars
RUN R -e 'install.packages(c("wk", "bit64", "Rcpp", "testthat"), repos = "https://cloud.r-project.org")'

CMD R CMD INSTALL /s2 --preclean && R -e 'testthat::test_local("/s2")'
