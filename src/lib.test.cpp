#include <gtest/gtest.h>

#ifdef TEMPLATE_MODULE
import lib;
#else
#include "lib.hpp"
#endif // TEMPLATE_MODULE

TEST(Simply, TheTruth) {
  ASSERT_TRUE(lib::return_true()) << "return_true is not the truth"; 
}
