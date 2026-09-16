# Retain vcpkg's MinGW platform setup while forcing the compilers selected by
# R. This supports both GCC-based Rtools and the Windows arm64 LLVM toolchain.
if(DEFINED Z_VCPKG_ROOT_DIR)
  set(R_S2_VCPKG_ROOT "${Z_VCPKG_ROOT_DIR}")
else()
  set(R_S2_VCPKG_ROOT "${_VCPKG_ROOT_DIR}")
endif()
include("${R_S2_VCPKG_ROOT}/scripts/toolchains/mingw.cmake")

set(CMAKE_C_COMPILER "$ENV{S2_R_CC}" CACHE FILEPATH "" FORCE)
set(CMAKE_CXX_COMPILER "$ENV{S2_R_CXX}" CACHE FILEPATH "" FORCE)

# R's compiler executables already target the correct Windows ABI. vcpkg's
# generic MinGW toolchain sets --target for Clang, while GCC rejects it.
unset(CMAKE_C_COMPILER_TARGET CACHE)
unset(CMAKE_CXX_COMPILER_TARGET CACHE)
