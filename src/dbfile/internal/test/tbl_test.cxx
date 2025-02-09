#include "sizes.hxx"
#include <gtest/gtest.h>
#ifdef ENABLE_MODULES
#ifndef IMPORT_STD
#include <cassert>
#include <sstream>
#else
import std;
#endif // !IMPORT_STD
import tinydb.dbfile.internal.tbl;
import tinydb.dbfile.coltype;
#else
#include "dbfile/coltype.hxx"
#include "dbfile/internal/tbl.hxx"
#endif // ENABLE_MODULES
TEST(tbl, init) {
  using namespace tinydb::dbfile;
  using namespace tinydb::dbfile::internal;
  TableMeta tbltest{"test-tbl"};
  std::stringstream test_stream{std::string(2 * tinydb::SIZEOF_PAGE, '\0')};
  test_stream.exceptions(std::stringstream::failbit);
  // NOLINTBEGIN
  tbltest.add_column(ColumnMeta{.m_name{"col1"},
                                .m_type = column::ColType::Uint8,
                                .m_col_id = 1,
                                .m_offset = 0});
  tbltest.add_column(ColumnMeta{.m_name{"col2"},
                                .m_type = column::ColType::Text,
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
            column::type_id(column::ColType::Text));
}
