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
    // breakpoint here to inspect variables
    // if get_column or read_from is faulty, this part here throws an exception.
    auto initial_col2 = tbltest.get_column("col2").value().get();
    auto read_col2 = readtest.get_column("col2").value().get();

    ASSERT_EQ(initial_col2.m_name, read_col2.m_name);
    ASSERT_EQ(initial_col2.m_offset, read_col2.m_offset);
    ASSERT_EQ(column::type_id(initial_col2.m_type),
              column::type_id(read_col2.m_type));
    ASSERT_EQ(column::type_id(initial_col2.m_type),
              column::type_id(column::TextType{}));
    // here it's safe to yank out TextType in both of these columns.
    auto initial_txt = std::get<column::TextType>(initial_col2.m_type);
    auto read_txt = std::get<column::TextType>(read_col2.m_type);
    ASSERT_EQ(initial_txt.get_size(), read_txt.get_size());
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
