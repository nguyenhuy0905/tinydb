/**
 * @file page_meta.cxx
 * @brief Simply a module export for page_meta.hxx. If the project isn't built
 * with modules enabled, this file is not compiled.
 */

#ifdef ENABLE_MODULES
module;
#include "general/sizes.hxx"
#include <cassert>
#ifndef IMPORT_STD
#include <cstdint>
#include <utility>
#endif
export module tinydb.dbfile.internal.page:meta;
export import tinydb.dbfile.internal.page_base;
import tinydb.dbfile.internal.heap_base;
#ifdef IMPORT_STD
import std;
#endif

#include "dbfile/internal/page_meta.hxx"
#else
#endif // ENABLE_MODULES
