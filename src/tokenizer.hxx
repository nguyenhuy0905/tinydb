#ifndef TINYDB_TOKENIZER_HXX
#define TINYDB_TOKENIZER_HXX

#include <expected>
#include <optional>
#include <variant>
#include <vector>

namespace tinydb {

enum class ParseError : uint8_t {
    UnknownCommand = 0,
    MissingArguments,
    SussySymbols,
    EmptyQuote,
};

// explicit instantiation to reduce compile time.
using ParserReturn = std::expected<void, ParseError>;

/**
 * @brief Types of symbols recognized by the CLI.
 */
enum class Symbol : uint8_t {
    Equal,
    EqualEquals,
    LessThan,
    LessThanEqual,
    MoreThan,
    MoreThanEqual,
    Backslash,
};

enum class State : uint8_t {
    Word,
    Quote,
    Number,
    Equal,
    LessThan,
    MoreThan,
};

// explicit instantiation to reduce compile time.
using Token = std::variant<std::string, int64_t, Symbol>;

/**
 * @class Parser
 * @brief Parses an input string into tokens.
 *
 */
class Tokenizer {
  public:
    Tokenizer() = default;

    /**
     * @brief Take the list of tokens out of `Parser`.
     * After this function call, one should not use this `Parser` anymore, or
     * one should construct a new `Parser.`
     *
     * @return rvalue of tokens vector.
     */
    auto take_tokens() -> std::vector<Token>&& { return std::move(m_tokens); }

    /**
     * @brief Parses the input string.
     *
     * @param input
     * @return Nothing if successful, ParseError if an error occurs.
     */
    auto parse(std::string_view input) -> ParserReturn;

    /**
     * @brief Consumes the current tokenizer and returns its vector of tokens.
     */
    auto consume() -> std::vector<Token>&& {
        return std::move(m_tokens);
    }

    /**
     * @brief State machine.
     *
     * NOTE: We can actually skip this handle_state function for better runtime
     * (that is, each time we change state, we change the handle function
     * pointer also.)
     *
     * @param c Next character.
     * @return Whether parsing succeeded.
     */
    auto handle_state(char c) -> ParserReturn {
        switch (m_curr_state) {
        case State::Word:
            return handle_word(c);
        case State::Quote:
            return handle_quote(c);
        case State::Number:
            return handle_number(c);
        case State::Equal:
            return handle_equal(c);
        case State::LessThan:
            return handle_less_than(c);
        case State::MoreThan:
            return handle_more_than(c);
        default:
            return std::unexpected{ParseError::SussySymbols};
        }
        return {};
    }

    /**
     * @brief Transition for `State::Word`
     *
     * @param c Next character.
     */
    auto handle_word(char c) -> ParserReturn;

    /**
     * @brief Transition for `State::Quote`
     *
     * @param c Next character.
     */
    auto handle_quote(char c) -> ParserReturn;

    /**
     * @brief Transition for `State::Number`
     *
     * @param c Next character.
     */
    auto handle_number(char c) -> ParserReturn;

    /**
     * @brief Transition for `State::Equal`
     *
     * @param c Next character.
     */
    auto handle_equal(char c) -> ParserReturn;

    /**
     * @brief Transition for `State::LessThan`
     *
     * @param c Next character.
     */
    auto handle_less_than(char c) -> ParserReturn;

    /**
     * @brief Transition for `State::MoreThan`
     *
     * @param c Next character.
     */
    auto handle_more_than(char c) -> ParserReturn;

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
    std::optional<int64_t> m_curr_num;
};

} // namespace tinydb

#endif // !TINYDB_PARSER_HXX
