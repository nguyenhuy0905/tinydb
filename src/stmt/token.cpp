#ifdef TINYDB_MODULE
module;
#include <cstddef>
#include <cstdint>
#ifndef TINYDB_IMPORT_STD
#include <string>
#include <string_view>
#include <vector>
#endif // !TINYDB_IMPORT_STD
export module tinydb.stmt:token;
#ifdef TINYDB_IMPORT_STD
import std;
#endif // TINYDB_IMPORT_STD
#endif // TINYDB_MODULE
#include "token.hpp"
