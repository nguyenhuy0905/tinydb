#ifndef ENABLE_MODULE
#else
module;
#include <cstdint>
#include <cassert>
#include "general/sizes.hxx"
export module tinydb.dbfile.internal.page:meta;
export import :base;
#include "dbfile/internal/page_meta.hxx"
#endif // !ENABLE_MODULE
