
#include "absl/base/config.h"

#if defined(ABSL_LTS_RELEASE_VERSION) && ABSL_LTS_RELEASE_VERSION < 20230802L
#error "Abseil C++ version is too old"
#endif
