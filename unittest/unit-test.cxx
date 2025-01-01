#include "page.hxx"
#include <gtest/gtest.h>
#include <print>

TEST(some, obvious_thingy) { ASSERT_TRUE(1 == 1); }

TEST(page, init) {
    using namespace tinydb::dbfile;
    PageMeta init1{FreePageMeta{}};
    // TODO: use a stringstream or something to test I/O.
}
