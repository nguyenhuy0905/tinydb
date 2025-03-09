#include <gtest/gtest.h>

#ifdef TINYDB_MODULE
import lib;
#else
#include "tinydb.hpp"
#endif // TINYDB_MODULE

TEST(Simply, TheTruth) {
  ASSERT_TRUE(lib::return_true()) << "return_true is not the truth";
}
