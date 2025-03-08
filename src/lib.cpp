#ifdef TEMPLATE_MODULE
module;
export module lib;
export {
#endif
#include "lib.hpp"
#ifdef TEMPLATE_MODULE
}
#endif // TEMPLATE_MODULE

namespace lib {
  auto return_true() -> bool {
    return true;
  }
}
