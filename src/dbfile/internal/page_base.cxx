#ifdef ENABLE_MODULES
module;
#include "general/modules.hxx"
#include <cstdint>
#ifndef IMPORT_STD
#include <type_traits>
#endif
export module tinydb.dbfile.internal.page:base;
#ifdef IMPORT_STD
import std;
#endif
#include "dbfile/internal/page_base.hxx"
#endif // ENABLE_MODULES
