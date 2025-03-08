#include "cli.hxx"

auto main() -> int {
  auto cli = tinydb::cli::Cli{};
  cli.run();

  return 0;
}
