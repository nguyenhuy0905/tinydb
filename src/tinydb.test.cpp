#include <gtest/gtest.h>

#ifdef TINYDB_MODULE
import lib;
#else
#endif // TINYDB_MODULE

TEST(Placeholder, Test) {
  ASSERT_TRUE(1 == 1) << "Error, 1 is not equal to 1";
}
