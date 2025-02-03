/**
 * @file page.cxx
 * @brief Simply a module export for all page declarations/definitions. If the
 * project isn't built with modules enabled, this file is not compiled.
 */

#ifdef ENABLE_MODULES
export module tinydb.dbfile.internal.page;
export import :base;
export import :meta;
export import :serialize;
#endif // ENABLE_MODULES
