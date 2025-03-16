#include <gtest/gtest.h>
#ifndef TINYDB_IMPORT_STD
#include <string_view>
#endif
#ifdef TINYDB_MODULE
import tinydb.stmt;
#ifdef TINYDB_IMPORT_STD
import std;
#endif
#else
#include "token.detail.hpp"
#endif

using namespace std::literals;
using namespace tinydb::stmt;

TEST(Tokenizer, Empty) {
  TokenizerData test_data{""sv};
  Tokenizer tok{};
  auto tok_result = tok(test_data);
  EXPECT_FALSE(tok_result);
  EXPECT_EQ(tok_result.error().type, ParseError::ErrType::UnendedStmt);
  EXPECT_TRUE(std::move(test_data).move_token_list().empty());
}

TEST(Tokenizer, OneNum) {
  TokenizerData test_data{"34;"sv};
  Tokenizer tok{};
  auto tok_result = [&]() {
    auto ret = tok(test_data);
    while (ret) {
      ret = (*ret)(test_data);
    }
    return ret;
  }();
  auto tok_list = std::move(test_data).move_token_list();
  EXPECT_FALSE(tok_list.empty());
  EXPECT_EQ(tok_list.front().lexeme, "34"sv);
  EXPECT_EQ(tok_list.front().type, TokenType::Number);
  EXPECT_FALSE(tok_result);
  EXPECT_EQ(tok_result.error().type, ParseError::ErrType::Done);
}
