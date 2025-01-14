#ifndef ENABLE_MODULE
#include "dbfile.hxx"
#include "tbl.hxx"
#include <gtest/gtest.h>
#include <sstream>
TEST(dbfile, init) {
    using namespace tinydb::dbfile;
    auto test_stream = std::make_unique<std::stringstream>(
        std::string(2 * tinydb::SIZEOF_PAGE, '\0'));
    test_stream->exceptions(std::stringstream::failbit);

    // i got an exception trying to write into this without creating and writing
    // the table in first. Which is actually a good thing. Better than some
    // random data being written.
    TableMeta meta{"tbl1"};
    meta.add_column(
        ColumnMeta{.m_name{"col1"}, .m_size = 4, .m_col_id = 1, .m_offset = 0});
    meta.set_key("col1");
    meta.write_to(*test_stream);
    auto dbfile = DbFile::construct_from(std::move(test_stream));
    // create a breakpoint here and check.
    // You know, I don't want to provide an equality operator just to assert.
}
#else
import tinydb.dbfile;
// #include "sizes.hxx"
#endif // !ENABLE_MODULE
