
#include <gtest/gtest.h>

#include "s2geography.h"

using namespace s2geography;

TEST(Distance, PointDistance) {
  WKTReader reader;
  auto geog1 = reader.read_feature("POINT (0 0)");
  auto geog2 = reader.read_feature("POINT (90 0)");
  ShapeIndexGeography geog1_index(*geog1);
  ShapeIndexGeography geog2_index(*geog2);

  EXPECT_DOUBLE_EQ(s2_distance(geog1_index, geog2_index), M_PI / 2);
}
