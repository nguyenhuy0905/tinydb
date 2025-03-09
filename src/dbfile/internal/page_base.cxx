/**
 * @file page_base.cxx
 * @brief Simply a module export for page_base.hxx. Won't be compiled if the
 * project is built without modules enabled.
 */

#ifdef ENABLE_MODULES
module;
#include <cstdint>
#ifndef IMPORT_STD
#include <type_traits>
#endif
export module tinydb.dbfile.internal.page_base;
#ifdef IMPORT_STD
import std;
#endif
#include "dbfile/internal/page_base.hxx"
#endif // ENABLE_MODULES
