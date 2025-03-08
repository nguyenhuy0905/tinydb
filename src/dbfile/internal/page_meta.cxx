/**
 * @file page_meta.cxx
 * @brief Simply a module export for page_meta.hxx. If the project isn't built
 * with modules enabled, this file is not compiled.
 */

#ifdef ENABLE_MODULES
module;
#include "general/modules.hxx"
#include "general/sizes.hxx"
#include "general/utils.hxx"
#include <cassert>
#ifndef IMPORT_STD
#include <cstdint>
#include <utility>
#endif
export module tinydb.dbfile.internal.page:meta;
import tinydb.dbfile.internal.heap.base;
export import :base;
#ifdef IMPORT_STD
import std;
#endif
#else
#endif // ENABLE_MODULES

#include "dbfile/internal/page_meta.hxx"
