// If this #include goes after the module include, it breaks.
#include <fmt/core.h>
#ifdef TINYDB_MODULE
import tinydb.stmt;
#ifdef TINYDB_IMPORT_STD
import std;
#endif // TINYDB_IMPORT_STD
#else
#endif // TINYDB_MODULE
// #include "repl/stmt.hpp"

auto main([[maybe_unused]] int argc, [[maybe_unused]] char *argv[]) -> int {
  
}
