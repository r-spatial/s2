cmake_minimum_required(VERSION 3.14)

project(r_s2geography LANGUAGES CXX)

find_package(s2 CONFIG REQUIRED)

add_library(
  s2geography STATIC
  ../src/s2geography/accessors-geog.cc
  ../src/s2geography/accessors.cc
  ../src/s2geography/build.cc
  ../src/s2geography/coverings.cc
  ../src/s2geography/distance.cc
  ../src/s2geography/geography.cc
  ../src/s2geography/linear-referencing.cc
  ../src/s2geography/predicates.cc)

set_target_properties(s2geography PROPERTIES POSITION_INDEPENDENT_CODE ON)
target_link_libraries(s2geography PUBLIC s2::s2)

# The R sources must use the same S2 headers as this library. Generate a shell-
# and Make-compatible include fragment from the imported CMake target.
file(
  GENERATE
  OUTPUT "${CMAKE_BINARY_DIR}/s2_include_flags.txt"
  CONTENT "-isystem $<JOIN:$<REMOVE_DUPLICATES:$<TARGET_PROPERTY:s2::s2,INTERFACE_INCLUDE_DIRECTORIES>>, -isystem >\n")

# Abseil is split into many interdependent libraries whose names change between
# releases. Record the link interface resolved by CMake instead of duplicating
# that dependency list in configure. This is the non-MSVC form of the approach
# used by Apache Sedona's sedona-s2geography crate.
set(CMAKE_ECHO_LINK_EXECUTABLE
    "sh -c \"echo <LINK_LIBRARIES> > <TARGET>\"")

add_executable(linker_flags CMakeLists.txt)
add_dependencies(linker_flags s2geography)
target_link_libraries(linker_flags PRIVATE "$<TARGET_FILE:s2geography>" s2::s2)
set_target_properties(linker_flags PROPERTIES LINKER_LANGUAGE ECHO SUFFIX ".txt")
