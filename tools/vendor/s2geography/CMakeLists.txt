
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

# openssl on Apple / Homebrew (dependency via s2geometry)
# copied from Arrow
# (copyright The Apache Software Foundation, Apache License, Version 2.0)

if(APPLE AND NOT OPENSSL_ROOT_DIR)
  find_program(BREW_PROG brew)
  if(BREW_PROG)
    execute_process(COMMAND ${BREW_PROG} --prefix "openssl@1.1"
                    OUTPUT_VARIABLE OPENSSL11_BREW_PREFIX
                    OUTPUT_STRIP_TRAILING_WHITESPACE)
    if(OPENSSL11_BREW_PREFIX)
      set(OPENSSL_ROOT_DIR ${OPENSSL11_BREW_PREFIX})
    else()
      execute_process(COMMAND ${BREW_PROG} --prefix "openssl"
                      OUTPUT_VARIABLE OPENSSL_BREW_PREFIX
                      OUTPUT_STRIP_TRAILING_WHITESPACE)
      if(OPENSSL_BREW_PREFIX)
        set(OPENSSL_ROOT_DIR ${OPENSSL_BREW_PREFIX})
      endif()
    endif()
  endif()
endif()

# s2geometry

if(DEFINED ENV{CONDA_PREFIX})
  set(S2GEOGRAPHY_S2_SOURCE_DEFAULT "CONDA")
else()
  set(S2GEOGRAPHY_S2_SOURCE_DEFAULT "AUTO")
endif()

set(S2GEOGRAPHY_S2_SOURCE
  "${S2GEOGRAPHY_S2_SOURCE_DEFAULT}"
  CACHE STRING "Method to use for acquiring the s2geometry dependency")

message(STATUS "Using ${S2GEOGRAPHY_S2_SOURCE} approach to find s2geometry")

macro(build_s2)
  message(STATUS "Building s2geometry from source (version 0.10.0)")

  FetchContent_Declare(
    s2
    GIT_REPOSITORY https://github.com/google/s2geometry.git
    GIT_TAG tags/v0.10.0
    GIT_SHALLOW TRUE)
  FetchContent_MakeAvailable(s2)

  set_property(TARGET s2 PROPERTY CXX_STANDARD ${CMAKE_CXX_STANDARD})

  # this might be needed since s2geometry includes it in general
  # but not for any target explicilty?
  find_package(OpenSSL)
  target_include_directories(s2 INTERFACE ${OPENSSL_INCLUDE_DIR})
endmacro()

if(${S2GEOGRAPHY_S2_SOURCE} STREQUAL "CONDA")
  set(S2_ROOT_DIR "$ENV{CONDA_PREFIX}")
  set(S2_SOURCE "SYSTEM")
elseif(${S2GEOGRAPHY_S2_SOURCE} STREQUAL "BREW")
  # required for Homebrew installed s2geometry headers to find OpenSSL headers
  find_package(OpenSSL)
  include_directories(${OPENSSL_INCLUDE_DIR})
  set(S2_SOURCE "SYSTEM")
else()
  set(S2_SOURCE ${S2GEOGRAPHY_S2_SOURCE})
endif()

if(${S2_SOURCE} STREQUAL "AUTO")
  find_package(s2 QUIET)
  if(${s2_FOUND})
    message(STATUS "Found s2: ${s2_INCLUDE_DIRS}")
  else()
    build_s2()
  endif()
elseif(${S2_SOURCE} STREQUAL "BUNDLED")
  build_s2()
elseif(${S2_SOURCE} STREQUAL "SYSTEM")
  find_package(s2 REQUIRED)
  if(NOT ${s2_FOUND})
    message(FATAL_ERROR "Couldn't find s2geometry")
  endif()
endif()

if (MSVC AND NOT ${S2_SOURCE} STREQUAL "BUNDLED")
  # used in s2geometry's CMakeLists.txt but not defined in target
  # TODO: move this in FindS2.cmake?
  target_compile_definitions(s2 INTERFACE _USE_MATH_DEFINES)
  target_compile_definitions(s2 INTERFACE NOMINMAX)
  target_compile_options(s2 INTERFACE /J)
endif()

# Abseil (bundled build not supported)

find_package(absl REQUIRED)
if(${absl_FOUND})
  get_target_property(ABSL_INCLUDE_DIRS absl::memory INTERFACE_INCLUDE_DIRECTORIES)
  message(STATUS "Found absl: ${ABSL_INCLUDE_DIRS}/absl")
else()
  message(FATAL_ERROR "Couldn't find absl")
endif()

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

