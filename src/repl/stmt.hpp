#ifndef TINYDB_REPL_STMT_HPP
#define TINYDB_REPL_STMT_HPP

#include "tinydb_export.h"
#ifndef TINYDB_MODULE
#include <memory>
#include <string_view>
#endif // !TINYDB_MODULE

namespace tinydb::repl {
class TINYDB_EXPORT Statement {
public:
  Statement() = delete;
  static auto parse(std::string_view t_sv) -> Statement;

private:
  class Ast;
  explicit Statement(Ast&& t_ast);
  std::unique_ptr<Ast> m_stmt;
};
} // namespace tinydb::repl

#endif // !TINYDB_REPL_STMT_HPP
