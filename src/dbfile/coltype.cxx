/**
 * @file coltype.cxx
 * @brief Simply a module export of coltype.hxx. If the project isn't built with
 * module support, this file isn't compiled.
 */

#ifdef ENABLE_MODULES
module;

#include <cstdint>
#ifndef IMPORT_STD
#include <functional>
#include <optional>
#include <type_traits>
#include <utility>
#include <variant>
#endif
export module tinydb.dbfile.coltype;
import tinydb.dbfile.internal.heap_base;
#ifdef IMPORT_STD
import std;
#endif
#include "coltype.hxx"
#endif // ENABLE_MODULES
