
#include <gtest/gtest.h>

#include "s2geography.h"

using namespace s2geography;

TEST(WKTWriter, SignificantDigits) {
  WKTReader reader;
  // Lat/lon is converted to XYZ here for the internals, so
  // we need to pick a value that will roundtrip with 16 digits of precision
  auto geog = reader.read_feature("POINT (0 3.333333333333334)");

  WKTWriter writer_default;
  EXPECT_EQ(writer_default.write_feature(*geog), "POINT (0 3.333333333333334)");

  WKTWriter writer_6digits(6);
  EXPECT_EQ(writer_6digits.write_feature(*geog), "POINT (0 3.33333)");
}
