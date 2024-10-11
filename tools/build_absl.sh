
# https://cran.r-project.org/doc/manuals/r-release/R-exts.html#Using-cmake

: ${R_HOME=`R RHOME`}
if test -z "${R_HOME}"; then
  echo "could not determine R_HOME"
  exit 1
fi

# Do our best to pass on the user MAKEFLAGS. This can result in much faster
# compilation of the vendored library.
MAKEFLAGS=`${R_HOME}/bin/Rscript -e 'readRenviron("~/.R/Makevars"); cat(Sys.getenv("MAKEFLAGS"))'`

if test -z "$MAKE"; then MAKE="`which make`"; fi
if ${MAKE} --version ; then
  echo "Using MAKE=$MAKE $MAKEFLAGS"
else
  echo "make not found"
  exit 1
fi

if test -z "$CMAKE"; then CMAKE="`which cmake`"; fi
if test -z "$CMAKE"; then CMAKE=/Applications/CMake.app/Contents/bin/cmake; fi
if "${CMAKE}" --version ; then
  echo "Using CMAKE=$CMAKE"
  echo "Using MAKE=$MAKE $MAKEFLAGS"
else
  echo "cmake not found"
  exit 1
fi

CC=`"${R_HOME}/bin/R" CMD config CC`
CXX="`${R_HOME}/bin/R CMD config CXX17` `${R_HOME}/bin/R CMD config CXX17STD`"
CFLAGS=`"${R_HOME}/bin/R" CMD config CFLAGS`
R_CPPFLAGS=`"${R_HOME}/bin/R" CMD config CPPFLAGS`
R_CXXFLAGS=`"${R_HOME}/bin/R" CMD config CXX17FLAGS`
LDFLAGS=`"${R_HOME}/bin/R" CMD config LDFLAGS`

CMAKE_INSTALL_PREFIX="$1"

if [ ! -d "tools/build/abseil-cpp" ]; then
    mkdir -p "tools/build/abseil-cpp"
fi

cd "tools/build/abseil-cpp"

"${CMAKE}" \
  -G "Unix Makefiles" \
  -DCMAKE_BUILD_TYPE=Release \
  -DCMAKE_EXPORT_COMPILE_COMMANDS=ON \
  -DCMAKE_COLOR_MAKEFILE=OFF \
  -DCMAKE_CXX_FLAGS_RELEASE="${R_CPPFLAGS} ${R_CXXFLAGS} ${WIN_CPPFLAGS}" \
  -DBUILD_SHARED_LIBS=OFF \
  -DCMAKE_POSITION_INDEPENDENT_CODE=ON \
  -DCMAKE_INSTALL_PREFIX="${CMAKE_INSTALL_PREFIX}" \
  -DCMAKE_CXX_STANDARD=17 \
  -DABSL_PROPAGATE_CXX_STD=ON \
  "../../vendor/abseil-cpp" &&
  ${MAKE} ${MAKEFLAGS} &&
  "${CMAKE}" --install .

cd ../../..
