#ifdef ENABLE_MODULE
import lib;
#else
#include "lib.hxx"
#endif
auto main() -> int {
    say_hello();
    return 0;
}
