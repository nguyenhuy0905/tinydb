#include "cli.hxx"
#include "tokenizer.hxx"
#include <iostream>
#include <print>
#include <string>

namespace tinydb {

Cli::Cli() = default;

void Cli::run() {
    std::print("> ");
    std::flush(std::cout);
    std::string input{};
    std::getline(std::cin, input);
    Tokenizer ps{};
    ps.parse(input);
#ifndef NDEBUG
    ps.print_tokens();
#endif // !NDEBUG
}

} // namespace tinydb
