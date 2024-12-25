#include "tbl.hxx"
#include <gtest/gtest.h>

TEST(some, obvious_thingy) { ASSERT_TRUE(1 == 1); }

TEST(some, tbl_test) {
    using namespace tinydb::dbfile;
    TableMeta tbl{"some-tbl"};
    tbl.add_column(
        ColumnMeta{.m_name{"col1"}, .m_size = 4, .m_col_id = 1, .m_offset = 0});
    tbl.add_column(
        ColumnMeta{.m_name{"col2"}, .m_size = 8, .m_col_id = 2, .m_offset = 4}); // NOLINT
    tbl.write_to({"test.db"});
    auto checkup = TableMeta::read_from("test.db");
}
