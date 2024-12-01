#ifndef TINYDB_AST_HXX
#define TINYDB_AST_HXX

#include "tokenizer.hxx"
#include <concepts>
#include <expected>
#include <memory>
#include <span>

namespace tinydb {

enum class ExprType : uint8_t;
// just a calculator for now

template <typename T>
concept Evaluable = requires(T a) {
    { a.evaluate() } -> std::same_as<Literal>;
};

/**
 * @class Expr
 * @brief Highest-level expression, which is the expression itself.
 *
 */
class Expr;

/**
 * @class Primary
 * @brief Either a literal value or a grouped expression.
 *  "number" | "(" Expr ")"
 */
class Primary {
  public:
  private:
    auto evaluate() -> Literal;
    std::variant<Literal, std::unique_ptr<Expr>> m_val;
};

class Unary {
  public:
    auto evaluate() -> Literal;

  private:
    enum class UnaryOp : uint8_t {
        Negate,
        Minus,
    };
    UnaryOp m_op;
};

class Ast {
  public:
    explicit Ast(std::span<Token> tokens);
    // void print_tree();
  private:
};

} // namespace tinydb

#endif // !TINYDB_AST_HXX
