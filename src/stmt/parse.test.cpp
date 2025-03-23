#include <gtest/gtest.h>
#ifndef TINYDB_IMPORT_STD
#else
import std;
#endif // !TINYDB_IMPORT_STD
#ifdef TINYDB_MODULE
import tinydb.stmt.parse;
#else
#include "parse.hpp"
#endif // TINYDB_MODULE
