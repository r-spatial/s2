
# s2geography

[![Run Tests](https://github.com/paleolimbot/s2geography/actions/workflows/build-and-run.yaml/badge.svg)](https://github.com/paleolimbot/s2geography/actions/workflows/build-and-run.yaml)
[![Codecov test coverage](https://codecov.io/gh/paleolimbot/s2geography/branch/master/graph/badge.svg)](https://app.codecov.io/gh/paleolimbot/s2geography?branch=master)

Google's [s2geometry](https://github.com/google/s2geometry) is a spherical geometry engine providing accurate and performant geometry operations for geometries on the sphere. This library provides a compatability layer on top of s2geometry for those more familiar with [simple features](https://en.wikipedia.org/wiki/Simple_Features), [GEOS](https://libgeos.org), and/or the [GEOS C API](https://libgeos.org/doxygen/geos__c_8h.html).

The s2geography library was refactored out of the [s2 package for R](https://github.com/r-spatial/s2), which has served as the backend for geometries with geographic coordinates in the popular [sf package for R](https://github.com/r-spatial/sf) since version 1.0.0. The library is currently under construction as it adapts to suit the needs of more than just a single R package. Suggestions to modify, replace, or completely rewrite this library are welcome!

## Example

A quick example (see also the `examples/` directory):

```cpp
#include <stdio.h>

#include <iostream>

#include "s2geography.h"

using namespace s2geography;

int main(int argc, char *argv[]) {
  WKTReader reader;
  std::unique_ptr<Geography> geog1 = reader.read_feature("POINT (-64 45)");
  std::unique_ptr<Geography> geog2 = reader.read_feature(
      "GEOMETRYCOLLECTION (POINT (30 10), LINESTRING (30 10, 10 30, 40 40), "
      "POLYGON ((30 10, 40 40, 20 40, 10 20, 30 10)))");

  ShapeIndexGeography geog1_index(*geog1);
  ShapeIndexGeography geog2_index(*geog2);

  double dist = s2_distance(geog1_index, geog2_index);

  printf("distance result is %g\n", dist);

  WKTWriter writer;
  std::cout << "geog1: " << writer.write_feature(*geog1) << "\n";
  std::cout << "geog2: " << writer.write_feature(*geog2) << "\n";
}
```

## Overview

The basic unit in s2geography is the `Geography` class. The three main subclasses of this wrap `std::vector<S2Point>`, `std::vector<std::unique_ptr<S2Polyline>>`, `std::unique_ptr<S2Polygon>`, and `std::vector<std::unique_ptr<Geography>>`; however, the `Geography` class is parameterized as zero or more `S2Shape` objects that also define an `S2Region`. This allows a flexible storage model (although only the four main subclasses have been tested).

Many operations in S2 require a `S2ShapeIndex` as input. This concept is similar to the GEOS prepared geometry and maps to the `ShapeIndexGeography` in this library. For indexing a vector of features, use the `GeographyIndex` (similar to the GEOS STRTree object).

The s2geography library sits on top of the s2geometry library, and you can and should use s2 directly!

## Build and Installation (from source)

There is no s2geography package available yet. If you want to use it, you need to build it from source. You can download the source by cloning this repository:

```bash
git clone https://github.com/paleolimbot/s2geography.git
```

### Requirements

- [CMake](https://cmake.org/)
- s2geometry
- [Abseil](https://github.com/abseil/abseil-cpp)
- OpenSSL (via s2geometry)

#### Conda

All the required dependencies above are available on conda-forge. You can install them using conda (or mamba):

```bash
conda install cmake libabseil s2geometry openssl -c conda-forge
```

#### Homebrew (MacOS)

Alternatively, you can install the required dependencies on MacOS with Homebrew:

``` bash
brew install cmake abseil s2geometry openssl
```

### Build using CMake

s2geography uses CMake to build the library. You first need to configure the build, e.g, using the following commands (from where the source has been downloaded or cloned):

```bash
mkdir build
cd build
cmake .. -DS2GEOGRAPHY_S2_SOURCE=AUTO -DCMAKE_CXX_STANDARD=17
cmake --build .
```

The CMake option `S2GEOGRAPHY_S2_SOURCE` specifies the method to use for acquiring s2geometry:

- `AUTO`: try to find s2geometry on the system default locations or download and build it from source if not found (default)
- `BUNDLED`: download and build s2geometry automatically from source
- `SYSTEM`: use s2geometry installed on one of the system default locations
- `CONDA`: use s2geometry installed in a conda environment (automatically selected when the environment is active)
- `BREW`: use s2geometry (and OpenSSL) installed with Homebrew

Note: s2geography does not support automatically acquiring and building Abseil and OpenSSL from source. If you don't have installed those libraries with conda or Homebrew, you might need to manually specify their location using the CMake options`absl_DIR` and `OPENSSL_ROOT_DIR`.

The CMake option `CMAKE_CXX_STANDARD` should be set according to the standard used to build Abseil and s2geometry (C++17 is set by default). 

The project is structured such that the VSCode CMake integration is triggered when the folder is open (if the default build doesn't work, consider adding `CMakeUserPresets.json` to configure things like the install directory, absl_DIR, or the location of OpenSSL).

### Install

After building the library, you can install it using:

```bash
cmake --install . --prefix ../dist
```

When building and installing in a conda environment, you can specify the conda environment's prefix location:

```bash
cmake --install . --prefix $CONDA_PREFIX
```

Or alternatively configure this by passing `-DCMAKE_INSTALL_PREFIX=$CONDA_PREFIX` when invoking cmake to configure the project.

## Development

s2geography provides units tests that can be built and run using [GTest](https://github.com/google/googletest). To enable it, use the CMake option `S2GEOGRAPHY_BUILD_TESTS=ON` (GTest will be downloaded and built automatically):

```bash
cmake .. -DS2GEOGRAPHY_BUILD_TESTS=ON
cmake --build .
```

You can then run the tests using `ctest`:

```bash
ctest -T test --output-on-failure .
```

You can also run specific tests using `ctest`'s `-R` flag:

```bash
ctest -T test . -R "Distance$"
```

s2geography also provides some examples that can be build using:

```bash
cmake .. -DS2GEOGRAPHY_BUILD_EXAMPLES=ON
cmake --build .
```

For VSCode users (with the C/C++ and CMake extensions), the CMakeUserPresets.json.example file shows a possible test/configuration preset that will build and run the tests and the examples.
