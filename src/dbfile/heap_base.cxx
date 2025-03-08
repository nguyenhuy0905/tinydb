#ifdef ENABLE_MODULES
module;
#include "general/utils.hxx"
#include "general/modules.hxx"
#ifndef IMPORT_STD
#include <cassert>
#include <utility>
#include <variant>
#endif // !IMPORT_STD
export module tinydb.dbfile.internal.heap.base;
#ifdef IMPORT_STD
import std;
#endif
#else
#endif // ENABLE_MODULES

#include "dbfile/heap_base.hxx"
