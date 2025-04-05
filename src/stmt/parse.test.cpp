#include <gtest/gtest.h>
#ifndef TINYDB_IMPORT_STD
#else
import std;
#endif // !TINYDB_IMPORT_STD
#ifdef TINYDB_MODULE
import tinydb.stmt.parse;
import tinydb.stmt.token;
#else
#include "parse.detail.hpp"
#include "parse.hpp"
#include <array>
#include <print>
#include <string_view>
#include <variant>
#endif // TINYDB_MODULE

using namespace tinydb::stmt;
using namespace std::literals;

TEST(Parse, Literals) {
  {
    NumberAst test_num{3};
    ASSERT_TRUE(std::holds_alternative<int64_t>(test_num.eval()));
    ASSERT_NO_THROW(std::get<int64_t>(test_num.eval()));
    ASSERT_EQ(std::get<int64_t>(test_num.eval()), 3);
    ASSERT_EQ(test_num.format(), "(lit-num: 3)"sv);
    auto test_num_eval = std::get<int64_t>(test_num.eval());
    Ast test{test_num};
    ASSERT_NO_THROW(std::get<int64_t>(test.do_eval()));
    auto test_eval = std::get<int64_t>(test.do_eval());
    ASSERT_EQ(test_num_eval, test_eval);
  }
  {
    std::string some_str{"Hello World"};
    // if something fumbles, it would not compile
    std::array test_strs{StrAst{std::string{"Hello World"}},
                         StrAst{R"(Hello World)"}, StrAst{some_str},
                         StrAst{std::string_view{some_str}},
                         // StrAst owns its string.
                         StrAst{std::move(some_str)}};
    for (auto &ast_str : test_strs) {
      ASSERT_TRUE(std::holds_alternative<std::string>(ast_str.eval()));
      ASSERT_NO_THROW(std::get<std::string>(ast_str.eval()));
      ASSERT_EQ(std::get<std::string>(ast_str.eval()), "Hello World"sv);
    }
  }
}

TEST(Parse, Unary) {
  {
    NumberAst test_num{3};
    UnaryExprAst test_un{UnaryExprAst::UnOp::Minus, LitExprAst{test_num}};
    ASSERT_TRUE(std::holds_alternative<int64_t>(test_un.eval()));
    auto eval_res = std::get<int64_t>(test_un.eval());
    ASSERT_EQ(eval_res, -3);
    Ast ast_test{test_un};
    ASSERT_TRUE(std::holds_alternative<int64_t>(ast_test.do_eval()));
    eval_res = std::get<int64_t>(ast_test.do_eval());
    ASSERT_EQ(eval_res, -3);
  }
}

TEST(Parse, Mul) {
  {
    MulExprAst test_mul{
        UnaryExprAst{UnaryExprAst::UnOp::Minus, LitExprAst{NumberAst{3}}}};
    ASSERT_TRUE(std::holds_alternative<int64_t>(test_mul.eval()));
    ASSERT_EQ(std::get<int64_t>(test_mul.eval()), -3);
    Ast ast_test{test_mul};
    ASSERT_TRUE(std::holds_alternative<int64_t>(ast_test.do_eval()));
    ASSERT_EQ(std::get<int64_t>(ast_test.do_eval()), -3);
  }
  {
    using enum MulExprAst::MulOp;
    using enum UnaryExprAst::UnOp;
    MulExprAst test_mul{
        UnaryExprAst{Plus, LitExprAst{NumberAst{2}}},
        std::vector{
            std::pair{Mul, UnaryExprAst{Minus, LitExprAst{NumberAst{3}}}},
            {Div, UnaryExprAst{Minus, LitExprAst{NumberAst{3}}}},
            {Mul, UnaryExprAst{Plus, LitExprAst{NumberAst{4}}}},
        },
    };
    ASSERT_TRUE(std::holds_alternative<int64_t>(test_mul.eval()));
    ASSERT_EQ(std::get<int64_t>(test_mul.eval()), 8);
    // j4f
    std::println("{}", test_mul.format());
  }
}

TEST(Parse, Plus) {
  {
    using enum MulExprAst::MulOp;
    MulExprAst test_mul{
        UnaryExprAst{UnaryExprAst::UnOp::Plus, LitExprAst{NumberAst{2}}},
        std::vector{
            std::pair{Mul, UnaryExprAst{UnaryExprAst::UnOp::Minus,
                                        LitExprAst{NumberAst{3}}}},

            {Div,
             UnaryExprAst{UnaryExprAst::UnOp::Minus, LitExprAst{NumberAst{3}}}},
            {Mul,
             UnaryExprAst{UnaryExprAst::UnOp::Plus, LitExprAst{NumberAst{4}}}},
        },
    };
    AddExprAst test_add{
        test_mul,
        std::initializer_list{std::pair{AddExprAst::AddOp::Add, test_mul},
                              {AddExprAst::AddOp::Sub, test_mul}}};
    ASSERT_TRUE(std::holds_alternative<int64_t>(test_add.eval()));
    ASSERT_EQ(std::get<int64_t>(test_add.eval()), 8);
    // j4f
    std::println("{}", test_add.format());
  }
}

TEST(Parse, Actual) {
  using enum TokenType;
  {
    std::vector num_tokens{Token{.lexeme{"-"sv}, .line = 0, .type = Minus},
                           Token{.lexeme{"34"sv}, .line = 0, .type = Number},
                           {.lexeme{";"sv}, .line = 0, .type = Semicolon}};
    auto parse_ret = parse(num_tokens);
    ASSERT_TRUE(parse_ret);
    auto eval_res = parse_ret->ast.do_eval();
    ASSERT_TRUE(std::holds_alternative<int64_t>(eval_res));
    ASSERT_EQ(std::get<int64_t>(eval_res), -34);
  }
  {
    // -34 + (-7 * 5);
    std::vector add_expr_tokens{Token{.lexeme{"-"sv}, .line = 0, .type = Minus},
                                {.lexeme{"34"sv}, .line = 0, .type = Number},
                                {.lexeme{"+"sv}, .line = 0, .type = Plus},
                                {.lexeme{"("sv}, .line = 0, .type = LeftParen},
                                {.lexeme{"-"sv}, .line = 0, .type = Minus},
                                {.lexeme{"7"sv}, .line = 0, .type = Number},
                                {.lexeme{"*"sv}, .line = 0, .type = Star},
                                {.lexeme{"5"sv}, .line = 0, .type = Number},
                                {.lexeme{")"sv}, .line = 0, .type = RightParen},
                                {.lexeme{";"sv}, .line = 0, .type = Semicolon}};
    auto parse_ret = parse(add_expr_tokens);
    ASSERT_TRUE(parse_ret);
    auto eval_res = parse_ret->ast.do_eval();
    ASSERT_TRUE(std::holds_alternative<int64_t>(eval_res));
    ASSERT_EQ(std::get<int64_t>(eval_res), -69);
  }
  {
    // -34 + -7 * 5;
    std::vector add_expr_tokens{Token{.lexeme{"-"sv}, .line = 0, .type = Minus},
                                {.lexeme{"34"sv}, .line = 0, .type = Number},
                                {.lexeme{"+"sv}, .line = 0, .type = Plus},
                                {.lexeme{"-"sv}, .line = 0, .type = Minus},
                                {.lexeme{"7"sv}, .line = 0, .type = Number},
                                {.lexeme{"*"sv}, .line = 0, .type = Star},
                                {.lexeme{"5"sv}, .line = 0, .type = Number},
                                {.lexeme{";"sv}, .line = 0, .type = Semicolon}};
    auto parse_ret = parse(add_expr_tokens);
    ASSERT_TRUE(parse_ret);
    auto eval_res = parse_ret->ast.do_eval();
    ASSERT_TRUE(std::holds_alternative<int64_t>(eval_res));
    ASSERT_EQ(std::get<int64_t>(eval_res), -69);
  }
}
