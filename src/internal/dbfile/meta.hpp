/**
 * @file meta.hpp
 * @brief Define the metadata at the beginning of a dbfile.
 *
 * The @ref Metadata structure doesn't contain everything inside a dbfile; only
 * useful information the program needs.
 */

#ifndef TINYDB_INTERNAL_DBFILE_META_HPP
#define TINYDB_INTERNAL_DBFILE_META_HPP

#ifndef TINYDB_MODULE
#include "internal/dbfile/defs.hpp"
#endif // !TINYDB_MODULE

#ifndef TINYDB_MODULE
namespace tinydb::internal::dbfile {
#else
export namespace tinydb::internal::dbfile {
#endif

struct Metadata {
  /**
   * @brief The number of pages currently in this dbfile.
   */
  page_num_t page_number;
};

}

#endif // !TINYDB_INTERNAL_DBFILE_META_HPP
