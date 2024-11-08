#ifdef ENABLE_MODULE
module;
#else
#include "lib.hxx"
#endif

#include "module_cfg.h"
#include <print>

#ifdef ENABLE_MODULE
export module lib;
#endif

TINYDB_EXPORT void say_hello() { // NOLINT
    std::print("Hello world\n");
}
