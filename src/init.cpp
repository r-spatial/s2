#include "absl/base/config.h"
#include "absl/log/log.h"
#include "s2/s2debug.h"
#include <Rcpp.h>
using namespace Rcpp;

// [[Rcpp::export]]
void cpp_s2_init() {
  // It's important to set this flag, as users might have "debug" flags
  // for their build environment, and there are some checks that will terminate
  // R instead of throw an exception if this value is set to true.
  // When possible, we also disable debug checks on a per-operation basis
  // if there is another way to do so (e.g., constructing S2Loop and S2Polygon objects).
  absl::SetFlag(&FLAGS_s2debug, false);
}

// Work around bug linking to gcc-built abseil-cpp from clang-built s2
// https://github.com/r-spatial/s2/issues/271
// https://github.com/abseil/abseil-cpp/issues/1747#issuecomment-2325811064
namespace absl {

ABSL_NAMESPACE_BEGIN

namespace log_internal {

template LogMessage& LogMessage::operator<<(const char& v);
template LogMessage& LogMessage::operator<<(const signed char& v);
template LogMessage& LogMessage::operator<<(const unsigned char& v);
template LogMessage& LogMessage::operator<<(const short& v);           // NOLINT
template LogMessage& LogMessage::operator<<(const unsigned short& v);  // NOLINT
template LogMessage& LogMessage::operator<<(const int& v);
template LogMessage& LogMessage::operator<<(const unsigned int& v);
template LogMessage& LogMessage::operator<<(const long& v);           // NOLINT
template LogMessage& LogMessage::operator<<(const unsigned long& v);  // NOLINT
template LogMessage& LogMessage::operator<<(const long long& v);      // NOLINT
template LogMessage& LogMessage::operator<<(
    const unsigned long long& v);  // NOLINT
template LogMessage& LogMessage::operator<<(void* const& v);
template LogMessage& LogMessage::operator<<(const void* const& v);
template LogMessage& LogMessage::operator<<(const float& v);
template LogMessage& LogMessage::operator<<(const double& v);
template LogMessage& LogMessage::operator<<(const bool& v);

}  // namespace log_internal

ABSL_NAMESPACE_END

}  // namespace absl
