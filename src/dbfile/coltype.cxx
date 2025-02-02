#ifdef ENABLE_MODULES
module;
#include "general/modules.hxx"
#include <cstdint>
#ifndef IMPORT_STD
#include <functional>
#include <optional>
#include <type_traits>
#include <utility>
#include <variant>
#endif
export module tinydb.dbfile.coltype;
#ifdef IMPORT_STD
import std;
#endif
#include "coltype.hxx"
#endif // ENABLE_MODULES
