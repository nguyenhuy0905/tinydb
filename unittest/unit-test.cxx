#include "page.hxx"
#include "tbl.hxx"
#include <fstream>
#include <gtest/gtest.h>
#include <print>

TEST(some, obvious_thingy) { ASSERT_TRUE(1 == 1); }

TEST(some, tbl_test) {
    std::println("This test is meant to be used with GDB");
    using namespace tinydb::dbfile;
    TableMeta tbl{"some-tbl"};
    std::fstream file{"test.db"};
    tbl.add_column(
        ColumnMeta{.m_name{"col1"}, .m_size = 4, .m_col_id = 1, .m_offset = 0});
    tbl.add_column(ColumnMeta{
        .m_name{"col2"}, .m_size = 8, .m_col_id = 2, .m_offset = 4}); // NOLINT
    ASSERT_FALSE(tbl.remove_column("hello"));
    ASSERT_TRUE(tbl.get_column("col1").has_value());
    ASSERT_TRUE(tbl.remove_column("col1"));
    tbl.write_to(file);
    auto checkup = TableMeta::read_from(file);
}

TEST(some, page_test) {
    std::println("This test is meant to be used with GDB");
    using namespace tinydb::dbfile;
    std::fstream file{"test.db"};

    FreePageMeta fpm{2, 1, 128}; // NOLINT(*magic-numbers)
    fpm.write_to(file);
    auto placeholder = FreePageMeta::placeholder(2);
    placeholder.read_from(file);
}
