#ifdef ENABLE_MODULES
module;
#include "general/modules.hxx"
#ifndef IMPORT_STD
#include <functional>
#include <string>
#include <string_view>
#include <unordered_map>
#endif // !IMPORT_STD
export module tinydb.dbfile.row_builder;
import tinydb.dbfile.coltype;
import tinydb.dbfile;
#ifdef IMPORT_STD
import std;
#endif // IMPORT_STD
#endif

#include "dbfile/row_builder.hxx"
