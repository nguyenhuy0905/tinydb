/**
 * @file tokenizer.hxx
 * @brief Tokenizing is the first step in lexical analysis.
 * This step simply translates a raw input strings into the following
 * categories:
 * - Spaces are ignored unless quoted.
 * - Symbols (comment, semicolon, comma and operators).
 * - Literal values (quoted string, number, boolean).
 * - Identifiers.
 *
 * The next step after tokenizing will resolve the Identifiers category
 * into either reserved keywords or actual identifiers.
 */

#ifndef TINYDB_TOKENIZER_HXX
#define TINYDB_TOKENIZER_HXX

#include <expected>
#include <variant>
#include <vector>

namespace tinydb {

enum class TokenizerError : uint8_t {
    UnknownCommand = 0,
    MissingArguments,
    SussySymbols,
    EmptyQuote,
};

/**
 * @brief Types of symbols recognized by the CLI.
 */
enum class Symbol : uint8_t {
    DashDash,      // --
    Equal,         // =
    NotEqual,      // <>
    LessThan,      // <
    LessThanEqual, // <=
    MoreThan,      // >
    MoreThanEqual, // >=
    Comma,         // ,
    Semicolon,     // ;
};

enum class State : uint8_t {
    Comment,
    Dash, // resolve to a comment (or a minus, in the future)
    Space,
    Word,
    Quote,
    LessThan,
    MoreThan,
};

// token types
/**
 * @class Identifier
 * @brief Generic unquoted bunch of characters
 */
struct Identifier {
    std::string val;
};
/**
 * @class Literal
 * @brief Literal values (string, number, boolean).
 * - Stored as a `variant<string, int32, bool>`.
 */
struct Literal {
    std::variant<std::string, int32_t, bool> val;
};

// explicit instantiation to reduce compile time.
using TokenizerReturn = std::expected<void, TokenizerError>;

// explicit instantiation to reduce compile time.
using StateHandleReturn = std::expected<State, TokenizerError>;

// explicit instantiation to reduce compile time.
using Token = std::variant<Identifier, Literal, Symbol>;

/**
 * @class Tokenizer
 * @brief Converts an input string into tokens.
 *
 */
class Tokenizer {
  public:
    Tokenizer() = default;

    /**
     * @brief Take the list of tokens out of `Tokenizer`.
     * After this function call, one should not use this `Tokenizer` anymore, or
     * one should construct a new `Tokenizer.`
     *
     * @return rvalue of tokens vector.
     */
    auto take_tokens() -> std::vector<Token>&& { return std::move(m_tokens); }

    /**
     * @brief Tokenize the input string.
     *
     * @param input
     * @return Nothing if successful, ParseError if an error occurs.
     */
    auto tokenize(std::string_view input) -> TokenizerReturn;

    auto handle_state(char c) -> StateHandleReturn {
        switch (m_curr_state) {
        case State::Space:
            return handle_space(c);
        case State::Word:
            return handle_word(c);
        case State::Quote:
            return handle_quote(c);
        // TODO: switch for all states
        default:
            return {};
        }
        return {};
    }

    auto handle_space(char c) -> StateHandleReturn;

    auto handle_word(char c) -> StateHandleReturn;

    auto handle_quote(char c) -> StateHandleReturn;

    // TODO: handle all the other equals

#ifndef NDEBUG
    /**
     * @brief Debug printing
     */
    void print_tokens();
#endif // !NDEBUG
  private:
    std::string m_curr_word{};
    std::vector<Token> m_tokens{};
    State m_curr_state{State::Word};
};

} // namespace tinydb

#endif // !TINYDB_TOKENIZER_HXX
