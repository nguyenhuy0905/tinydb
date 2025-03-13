// probably a clang bug, but if we #include after import std, we get a redecl
// error.
#include <gtest/gtest.h>
#ifdef TINYDB_MODULE
import tinydb;
#ifdef TINYDB_IMPORT_STD
import std;
#endif
#else
#endif // TINYDB_MODULE

TEST(Placeholder, Test) {
  ASSERT_TRUE(1 == 1) << "Error, 1 is not equal to 1";
}
