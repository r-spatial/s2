
cmake_minimum_required(VERSION 3.11)

include(FetchContent)
include(GNUInstallDirs)

message(STATUS "Building using CMake version: ${CMAKE_VERSION}")

project(S2Geography)

set(CMAKE_CXX_STANDARD 17 CACHE STRING "The C++ standard to build with")
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/third_party/cmake")

option(S2GEOGRAPHY_BUILD_TESTS "Build tests" OFF)
option(S2GEOGRAPHY_CODE_COVERAGE "Enable coverage reporting" OFF)
option(S2GEOGRAPHY_BUILD_EXAMPLES "Build s2geography examples" OFF)
option(BUILD_SHARED_LIBS "Build using shared libraries" ON)

# Dependencies
# ------------

find_package(OpenSSL REQUIRED)

# s2geometry

find_package(s2 REQUIRED)
message(STATUS "Found s2: ${s2_INCLUDE_DIRS}")

find_package(OpenSSL)
target_include_directories(s2 INTERFACE ${OPENSSL_INCLUDE_DIR})

# Export the include and lib directories
file(WRITE ${CMAKE_BINARY_DIR}/OPENSSL_INCLUDE_DIR "${OPENSSL_INCLUDE_DIR}")
file(WRITE ${CMAKE_BINARY_DIR}/OPENSSL_CRYPTO_LIBRARY "${OPENSSL_CRYPTO_LIBRARY}")

# Abseil (bundled build not supported)

find_package(absl REQUIRED)
get_target_property(ABSL_INCLUDE_DIRS absl::memory INTERFACE_INCLUDE_DIRECTORIES)
message(STATUS "Found absl: ${ABSL_INCLUDE_DIRS}/absl")

# GTest (always bundled)

if(S2GEOGRAPHY_BUILD_TESTS)
  FetchContent_Declare(
    googletest
    GIT_REPOSITORY https://github.com/google/googletest
    GIT_TAG tags/release-1.12.0
    GIT_SHALLOW TRUE
    )

  # For Windows: Prevent overriding the parent project's compiler/linker settings
  set(gtest_force_shared_crt ON CACHE BOOL "Always use msvcrt.dll" FORCE)

  FetchContent_MakeAvailable(googletest)

  # do not install googletest by default when running cmake --install
  set_property(DIRECTORY ${googletest_SOURCE_DIR} PROPERTY EXCLUDE_FROM_ALL YES)

  if(S2GEOGRAPHY_CODE_COVERAGE)
    add_library(coverage_config INTERFACE)
  endif()
endif()

# Build s2geography
# -----------------

include_directories(src)

add_library(s2geography
    src/s2geography/accessors-geog.cc
    src/s2geography/coverings.cc
    src/s2geography/linear-referencing.cc
    src/s2geography/accessors.cc
    src/s2geography/distance.cc
    src/s2geography/predicates.cc
    src/s2geography/build.cc
    src/s2geography/geography.cc
    src/s2geography/wkt-reader.cc
    src/s2geography/wkt-writer.cc)

set_target_properties(s2geography PROPERTIES
    POSITION_INDEPENDENT_CODE ${BUILD_SHARED_LIBS})

target_link_libraries(s2geography s2 absl::memory absl::str_format)

if(S2GEOGRAPHY_BUILD_TESTS)
  if(CMAKE_CXX_COMPILER_ID MATCHES "GNU" OR
     CMAKE_CXX_COMPILER_ID MATCHES "Clang" OR
     (CMAKE_CXX_COMPILER_ID MATCHES "Intel" AND NOT WIN32))
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wextra -Wreorder")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wunused-variable -Wunused-parameter")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wconversion -Wold-style-cast -Wno-sign-conversion")
  elseif(CMAKE_CXX_COMPILER_ID MATCHES "MSVC")
    add_definitions(-D_CRT_SECURE_NO_WARNINGS)
    add_definitions(-D_SILENCE_TR1_NAMESPACE_DEPRECATION_WARNING)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /EHsc /MP")
    set(CMAKE_EXE_LINKER_FLAGS /MANIFEST:NO)
  endif()

  enable_testing()

  add_executable(distance_test src/s2geography/distance_test.cc)
  add_executable(wkt_writer_test src/s2geography/wkt-writer_test.cc)

  if (S2GEOGRAPHY_CODE_COVERAGE)
    target_compile_options(coverage_config INTERFACE -O0 -g --coverage)
    target_link_options(coverage_config INTERFACE --coverage)
    target_link_libraries(s2geography coverage_config)
  endif()

  target_link_libraries(distance_test s2geography GTest::gtest_main)
  target_link_libraries(wkt_writer_test s2geography GTest::gtest_main)

  include(GoogleTest)
  gtest_discover_tests(distance_test)
  gtest_discover_tests(wkt_writer_test)
endif()

if(S2GEOGRAPHY_BUILD_EXAMPLES)
  add_executable(example-simple examples/example-simple/example-simple.cc)
  target_link_libraries(example-simple PUBLIC s2geography s2)
endif()

# Install s2geography
# -------------------

install(TARGETS s2geography
  RUNTIME DESTINATION "${CMAKE_INSTALL_BINDIR}"
  ARCHIVE DESTINATION "${CMAKE_INSTALL_LIBDIR}"
  LIBRARY DESTINATION "${CMAKE_INSTALL_LIBDIR}")

install(DIRECTORY src/ DESTINATION include FILES_MATCHING PATTERN "*.h")

if(S2GEOGRAPHY_BUILD_EXAMPLES)
  install(TARGETS example-simple DESTINATION examples)
endif()

