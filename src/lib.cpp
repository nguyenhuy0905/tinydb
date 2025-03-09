#ifdef TINYDB_MODULE
module;
export module lib;
export {
#endif
#include "lib.hpp"
#ifdef TINYDB_MODULE
}
#endif // TINYDB_MODULE

namespace lib {
  auto return_true() -> bool {
    return true;
  }
}
