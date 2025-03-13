#ifdef TINYDB_MODULE
module;
#include <cstddef>
#include <cstdint>
#ifndef TINYDB_IMPORT_STD
#include <expected>
#include <memory>
#include <span>
#include <string_view>
#include <vector>
#else
import std;
#endif // !TINYDB_IMPORT_STD
export module tinydb.stmt;
export import :token;
#endif // TINYDB_MODULE
#include "stmt.hpp"
