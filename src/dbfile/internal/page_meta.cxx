#ifdef ENABLE_MODULES
module;
#include "general/modules.hxx"
#include "general/sizes.hxx"
#include <cassert>
#ifndef IMPORT_STD
#include <cstdint>
#include <utility>
#endif
export module tinydb.dbfile.internal.page:meta;
export import :base;
#ifdef IMPORT_STD
import std;
#endif
#else
#include "dbfile/internal/page_base.hxx"
#include "general/sizes.hxx"
#include <cstdint>
#include <utility>
#include <cassert>
#endif // ENABLE_MODULES

#include "dbfile/internal/page_meta.hxx"
