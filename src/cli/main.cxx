#include "cli.hxx"

auto main() -> int {
    tinydb::Cli cli = tinydb::Cli{};
    cli.run();

    return 0;
}
