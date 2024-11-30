#ifndef TINYDB_AST_HXX
#define TINYDB_AST_HXX

#include <expected>
#include <span>
#include "tokenizer.hxx"

namespace tinydb {

// just a calculator for now

class Ast {
    public:
        void eval_expr(std::span<Token> tokens);
    private:
};
 
}

#endif // !TINYDB_AST_HXX
