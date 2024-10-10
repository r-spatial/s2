
ARG IMAGE=debian:testing
ARG ABSL_DEP=libabsl-dev

FROM ${IMAGE}

ENV TZ=UTC
RUN apt-get update && DEBIAN_FRONTEND=noninteractive apt-get install -y \
    r-base cmake \
    libudunits2-dev libgdal-dev libgeos-dev libproj-dev \
    libcurl4-openssl-dev libssl-dev ${ABSL_DEP}

# Make sure we can use all cores to install things
RUN mkdir ~/.R && echo "MAKEFLAGS = -j$(nproc)" > ~/.R/Makevars
RUN R -e 'install.packages(c("wk", "Rcpp", "testthat"), repos = "https://cloud.r-project.org")'
