#ifndef ENABLE_MODULE
#else
module;
#include <cstdint>
export module tinydb.dbfile.internal.page:impl;
export import :base;
#include "dbfile/internal/page_impl.hxx"
#endif // !ENABLE_MODULE
