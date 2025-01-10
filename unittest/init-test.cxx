#include "freelist.hxx"
#include "page.hxx"
#include <bit>
#include <gtest/gtest.h>

TEST(simple_page, init) {
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

TEST(free_list, init) {
    using namespace tinydb::dbfile;
    constexpr uint32_t filesize{2};
    std::stringstream test_stream{std::string(filesize * PAGESIZ, '\0')};
    constexpr uint16_t sizeoff{6};
    test_stream.seekp(sizeoff);
    test_stream.write(std::bit_cast<const char*>(&filesize), sizeof(filesize));
    // hook to GDB to check. These 2 should be the same.
    [[maybe_unused]] auto freelist = FreeListMeta::default_init(1, test_stream);
    [[maybe_unused]] auto f2 = FreeListMeta::construct_from(test_stream);

    // hook to GDB to check. This one's m_page_num should be 1.
    [[maybe_unused]] auto btlpage =
        freelist.allocate_page<BTreeLeafMeta>(test_stream);
    ASSERT_EQ(btlpage.get_pg_num(), 1);

    // filesize should be updated also.
    test_stream.seekg(sizeoff);
    test_stream.read(std::bit_cast<char*>(&filesize), sizeof(filesize));
    ASSERT_EQ(filesize, 3);
}
