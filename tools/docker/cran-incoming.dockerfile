FROM ghcr.io/r-devel/rcheckserver/debian

# Add WU Vienna AASC repository for R-devel
# RUN apt-get update && apt-get install -y curl gpg
# RUN curl -fsSL https://statmath.wu.ac.at/AASC/debian/keys/aasc-archive.pgp | gpg --dearmor -o /etc/apt/trusted.gpg.d/aasc.gpg
# RUN echo "deb https://statmath.wu.ac.at/AASC/debian testing main" > /etc/apt/sources.list.d/aasc.list
# RUN apt-get update && apt-get install -y rcheckserver

ENV R_VERSION=4.5.2

COPY tools/docker/Rconf /Rconf

RUN curl -O https://cran.rstudio.com/src/base/R-4/R-${R_VERSION}.tar.gz
RUN tar -xzvf R-${R_VERSION}.tar.gz
ENV R_SRC_DIR=R-${R_VERSION}

RUN chmod +x /Rconf && cd / && ./Rconf -x
RUN make && make install

# # Make sure we can use all cores to install things
RUN mkdir ~/.R && echo "MAKEFLAGS = -j$(nproc)" > ~/.R/Makevars
RUN R -e 'install.packages(c("wk", "bit64", "Rcpp", "testthat"), repos = "https://cloud.r-project.org")'

CMD R CMD INSTALL /s2 --preclean && R -e 'testthat::test_local("/s2")'
