#ifndef ENABLE_MODULE
#include "coltype.hxx"
#include "sizes.hxx"
#include "tbl.hxx"
#include <gtest/gtest.h>
#include <sstream>

TEST(tbl, init) {
    using namespace tinydb::dbfile;
    TableMeta tbltest{"test-tbl"};
    std::stringstream test_stream{std::string(2 * tinydb::SIZEOF_PAGE, '\0')};
    test_stream.exceptions(std::stringstream::failbit);
    // NOLINTBEGIN
    tbltest.add_column(ColumnMeta{.m_name{"col1"},
                                  .m_type{column::ScalarColType::Uint8},
                                  .m_col_id = 1,
                                  .m_offset = 0});
    tbltest.add_column(ColumnMeta{.m_name{"col2"},
                                  .m_type{column::TextType(128)},
                                  .m_col_id = 2,
                                  .m_offset = 1});
    // NOLINTEND
    tbltest.set_key("col1");
    tbltest.write_to(test_stream);
    auto readtest = TableMeta::read_from(test_stream);
    // breakpoint here and test
}
#else
import std;
import tinydb.dbfile.page;

/**
 * @brief Simply here while I'm finding a way to unit-test these modules.
 * @details I may have to move back to CTest.
 */
void something() {
    using namespace tinydb::dbfile;
    PageMeta t_meta{FreePageMeta{1}};
    std::println("{}", t_meta.get_pg_num());
}
#endif // !ENABLE_MODULE
