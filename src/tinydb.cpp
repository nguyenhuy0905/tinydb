#ifdef TINYDB_MODULE
module;
export module lib;
export {
#endif
#include "tinydb.hpp"
#ifdef TINYDB_MODULE
}
#endif // TINYDB_MODULE

namespace tinydb {
  auto return_true() -> bool {
    return true;
  }
}
