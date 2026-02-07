FROM ghcr.io/r-devel/rcheckserver/debian

ENV R_VERSION=4.5.2

RUN curl -O https://cran.rstudio.com/src/base/R-4/R-${R_VERSION}.tar.gz
RUN tar -xzvf R-${R_VERSION}.tar.gz
RUN (cd R-${R_VERSION}; \
  ./configure \
  --prefix=/opt/R/${R_VERSION} \
  --enable-R-shlib \
  --enable-memory-profiling \
  --with-blas \
  --with-lapack; \
  make; \
  make install \
  )

# Make sure we can use all cores to install things
RUN mkdir ~/.R && echo "MAKEFLAGS = -j$(nproc)" > ~/.R/Makevars
RUN R -e 'install.packages(c("wk", "bit64", "Rcpp", "testthat"), repos = "https://cloud.r-project.org")'

CMD R CMD INSTALL /s2 --preclean && R -e 'testthat::test_local("/s2")'
