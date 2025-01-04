#include "page.hxx"
#include <gtest/gtest.h>
#include <print>

TEST(some, obvious_thingy) { ASSERT_TRUE(1 == 1); }

TEST(page, init) {
    using namespace tinydb::dbfile;
    PageMeta init1{FreePageMeta{1, 2}};
    // if we don't fill in the string, stringstream will throw an error when we
    // seek to some position outside of the current string length.
    std::stringstream test_stream{std::string(2 * PAGESIZ, '\0')};
    test_stream.exceptions(std::stringstream::failbit);

    // hook this up into gdb to test the return value.
    init1.write_to(test_stream);
    auto init2 = PageMeta::construct_from<FreePageMeta>(test_stream, 1);

    // test cloning
    auto init3 = init1;
}
