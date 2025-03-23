#ifdef TINYDB_MODULE
module;
#include <cstddef>
#ifndef TINYDB_IMPORT_STD
#else
import std;
#endif // !TINYDB_IMPORT_STD
export module tinydb.stmt.parse;
import tinydb.stmt.token;
#else
#include "token.hpp"
#include <cstddef>
#endif // TINYDB_MODULE
#include "parse.hpp"
