
ARG IMAGE=alpine:latest

FROM ${IMAGE}

RUN apk add bash R R-dev abseil-cpp-dev

# Make sure we can use all cores to install things
RUN mkdir ~/.R && echo "MAKEFLAGS = -j$(nproc)" > ~/.R/Makevars
RUN R -e 'install.packages(c("wk", "bit64", "Rcpp", "testthat"), repos = "https://cloud.r-project.org")'

CMD R CMD INSTALL /s2 --preclean && R -e 'testthat::test_local("/s2")'
