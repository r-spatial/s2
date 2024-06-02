
# https://cran.r-project.org/doc/manuals/r-release/R-exts.html#Using-cmake

if test -z "$MAKE"; then MAKE="`which make`"; fi
if ${MAKE} --version ; then
  echo "Using MAKE=$MAKE $MAKEVARS"
else
  echo "cmake not found"
  exit 1
fi

if test -z "$CMAKE"; then CMAKE="`which cmake`"; fi
if test -z "$CMAKE"; then CMAKE=/Applications/CMake.app/Contents/bin/cmake; fi
if ${CMAKE} --version ; then
  echo "Using CMAKE=$CMAKE"
  echo "Using MAKE=$MAKE $MAKEVARS"
else
  echo "cmake not found"
  exit 1
fi

: ${R_HOME=`R RHOME`}
if test -z "${R_HOME}"; then
  echo "could not determine R_HOME"
  exit 1
fi

CC=`"${R_HOME}/bin/R" CMD config CC`
CXX=`${R_HOME}/bin/R CMD config CXX14`
CFLAGS=`"${R_HOME}/bin/R" CMD config CFLAGS`
CPPFLAGS=`"${R_HOME}/bin/R" CMD config CPPFLAGS`
CXXFLAGS=`"${R_HOME}/bin/R" CMD config CXX14FLAGS`
LDFLAGS=`"${R_HOME}/bin/R" CMD config LDFLAGS`

CMAKE_INSTALL_PREFIX="`pwd`/tools/dist"
CMAKE_PREFIX_PATH="`pwd`/tools/dist/lib/cmake:$CMAKE_PREFIX_PATH"

build_cmake () {
  if [ ! -d "tools/build/$1" ]; then
    mkdir -p "tools/build/$1"
  fi

  cd "tools/build/$1"

  ${CMAKE} \
    -DCMAKE_BUILD_TYPE=Release \
    -DBUILD_SHARED_LIBS=OFF \
    -DCMAKE_POSITION_INDEPENDENT_CODE=ON \
    -DCMAKE_INSTALL_PREFIX="${CMAKE_INSTALL_PREFIX}" \
    -DCMAKE_PREFIX_PATH="${CMAKE_PREFIX_PATH}" \
    -DCMAKE_OSX_DEPLOYMENT_TARGET=11.3 \
    -DCMAKE_CXX_STANDARD=14 \
    -DABSL_PROPAGATE_CXX_STD=ON \
    "../../vendor/$1" &&
    ${MAKE} ${MAKEVARS} &&
    ${CMAKE} --install .

  cd ../../..
}

build_cmake abseil-cpp
rm -rf tools/build/abseil-cpp
