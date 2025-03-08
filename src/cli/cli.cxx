#include "cli.hxx"
#include "tokenizer.hxx"
#include <iostream>
#include <print>
#include <string>

namespace tinydb::cli {

Cli::Cli() = default;

void Cli::run() {
  std::print("> ");
  std::flush(std::cout);
  std::string input{};
  std::getline(std::cin, input);
  Tokenizer tk{};
  tk.tokenize(input);
#ifndef NDEBUG
  tk.print_tokens();
#endif // !NDEBUG
}

} // namespace tinydb::cli
