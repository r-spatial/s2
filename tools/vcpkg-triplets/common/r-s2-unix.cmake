set(VCPKG_CRT_LINKAGE dynamic)
set(VCPKG_LIBRARY_LINKAGE static)
set(VCPKG_BUILD_TYPE release)

# Build vcpkg dependencies with the same toolchain and flags as R. Listing
# these as passthrough variables also makes them part of vcpkg's ABI hash.
set(VCPKG_ENV_PASSTHROUGH
    CC
    CXX
    CPPFLAGS
    CFLAGS
    CXXFLAGS
    LDFLAGS
    MACOSX_DEPLOYMENT_TARGET)

set(VCPKG_C_FLAGS_RELEASE "$ENV{CPPFLAGS} $ENV{CFLAGS}")
set(VCPKG_CXX_FLAGS_RELEASE "$ENV{CPPFLAGS} $ENV{CXXFLAGS}")
set(VCPKG_LINKER_FLAGS_RELEASE "$ENV{LDFLAGS}")
