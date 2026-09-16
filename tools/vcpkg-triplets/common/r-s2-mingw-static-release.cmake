set(VCPKG_CRT_LINKAGE dynamic)
set(VCPKG_LIBRARY_LINKAGE static)
set(VCPKG_BUILD_TYPE release)
set(VCPKG_CMAKE_SYSTEM_NAME MinGW)

# Build dependencies with exactly the compiler and flags reported by R. These
# variables are also included in vcpkg's ABI hash for its binary cache.
set(VCPKG_ENV_PASSTHROUGH
    PATH
    S2_R_CC
    S2_R_CXX
    S2_R_CFLAGS
    S2_R_CXXFLAGS
    S2_R_LDFLAGS)

set(VCPKG_C_FLAGS_RELEASE "$ENV{S2_R_CFLAGS}")
set(VCPKG_CXX_FLAGS_RELEASE "$ENV{S2_R_CXXFLAGS} -D_USE_MATH_DEFINES")
set(VCPKG_LINKER_FLAGS_RELEASE "$ENV{S2_R_LDFLAGS}")
set(VCPKG_CHAINLOAD_TOOLCHAIN_FILE
    "${CMAKE_CURRENT_LIST_DIR}/r-s2-rtools-mingw.cmake")
