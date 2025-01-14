#ifndef ENABLE_MODULE
#include "dbfile.hxx"
#include <gtest/gtest.h>
#include <sstream>
// TEST(dbfile, init) {
//     using namespace tinydb::dbfile;
//     auto test_stream = std::make_unique<std::stringstream>(
//         std::string(2 * tinydb::SIZEOF_PAGE, '\0'));
//     auto dbfile = DbFile::construct_from(std::move(test_stream));
// }
#else
import tinydb.dbfile;
// #include "sizes.hxx"
#endif // !ENABLE_MODULE

