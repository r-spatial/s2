
#include "s2geography/constructor.h"
#include "s2geography/geography.h"

namespace s2geography {

namespace util {

class WKTReaderInternal;

}

class WKTReader {
 public:
  WKTReader() : WKTReader(util::Constructor::Options()) {}
  WKTReader(const util::Constructor::Options& options);
  std::unique_ptr<Geography> read_feature(const char* text, int64_t size);
  std::unique_ptr<Geography> read_feature(const char* text);

 private:
  util::Constructor::Options options_;
  std::shared_ptr<util::WKTReaderInternal> reader_;
  std::unique_ptr<util::FeatureConstructor> constructor_;
};

}  // namespace s2geography
