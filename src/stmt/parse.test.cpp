#include <gtest/gtest.h>
#ifndef TINYDB_IMPORT_STD
#else
import std;
#endif // !TINYDB_IMPORT_STD
#ifdef TINYDB_MODULE
import tinydb.stmt.parse;
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
      ASSERT_EQ(ast_str.format(), R"((lit-str: Hello World))");
      ASSERT_EQ(std::get<std::string>(ast_str.eval()), "Hello World"sv);
    }
  }
}

TEST(Parse, Unary) {
  {
    NumberAst test_num{3};
    UnaryExprAst test_un{UnaryExprAst::UnaryOp::Minus, test_num};
    ASSERT_TRUE(std::holds_alternative<int64_t>(test_un.eval()));
    auto eval_res = std::get<int64_t>(test_un.eval());
    ASSERT_EQ(eval_res, -3);
    Ast ast_test{test_un};
    ASSERT_TRUE(std::holds_alternative<int64_t>(ast_test.do_eval()));
    eval_res = std::get<int64_t>(ast_test.do_eval());
    ASSERT_EQ(eval_res, -3);
    ASSERT_EQ(ast_test.do_format(),
              "(unary-expr: (unary-op: -) (lit-num: 3))"sv);
  }
}

TEST(Parse, Mul) {
  {
    MulExprAst test_mul{
        UnaryExprAst{UnaryExprAst::UnaryOp::Minus, NumberAst{3}}};
    ASSERT_TRUE(std::holds_alternative<int64_t>(test_mul.eval()));
    ASSERT_EQ(std::get<int64_t>(test_mul.eval()), -3);
    Ast ast_test{test_mul};
    ASSERT_TRUE(std::holds_alternative<int64_t>(ast_test.do_eval()));
    ASSERT_EQ(std::get<int64_t>(ast_test.do_eval()), -3);
  }
  {
    using enum MulExprAst::MulOp;
    using enum UnaryExprAst::UnaryOp;
    MulExprAst test_mul{
        UnaryExprAst{Plus, NumberAst{2}},
        std::initializer_list{
            std::pair{Multiply, UnaryExprAst{Minus, NumberAst{3}}},

            {Divide, {Minus, NumberAst{3}}},
            {Multiply, {Plus, NumberAst{4}}},
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
        UnaryExprAst{UnaryExprAst::UnaryOp::Plus, NumberAst{2}},
        std::initializer_list{
            std::pair{Multiply,
                      UnaryExprAst{UnaryExprAst::UnaryOp::Minus, NumberAst{3}}},

            {Divide, {UnaryExprAst::UnaryOp::Minus, NumberAst{3}}},
            {Multiply, {UnaryExprAst::UnaryOp::Plus, NumberAst{4}}},
        },
    };
    AddExprAst test_add{
        test_mul,
        std::initializer_list{std::pair{AddExprAst::AddOp::Plus, test_mul},
                              {AddExprAst::AddOp::Minus, test_mul}}};
    ASSERT_TRUE(std::holds_alternative<int64_t>(test_add.eval()));
    ASSERT_EQ(std::get<int64_t>(test_add.eval()), 8);
    // j4f
    std::println("{}", test_add.format());
  }
}
