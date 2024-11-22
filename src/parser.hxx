#ifndef TINYDB_PARSER_HXX
#define TINYDB_PARSER_HXX

#include "tokenizer.hxx"

namespace tinydb {

// TODO: define the AST class (or at least ASTNode)

/**
 * @class Parser
 * @brief Parses a list of tokens into an AST
 *
 */
class Parser {
  public:
    /**
     * @brief Parses the list of tokens into real shits that the (upcoming)
     * Interpreter can understand
     *
     * @param toks list of tokens. Take this using Tokenizer::consume I guess.
     */
    auto parse(const std::vector<Token>& toks);

  private:
};

} // namespace tinydb

#endif // !TINYDB_PARSER_HXX
