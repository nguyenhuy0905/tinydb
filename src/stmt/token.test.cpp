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

TEST(Tokenizer, Num) {
  {
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
  {
    TokenizerData test_data{"12.34;"sv};
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
    EXPECT_FALSE(tok_result);
    EXPECT_EQ(tok_result.error().type, ParseError::ErrType::Done);
    EXPECT_EQ(tok_list.front().lexeme, "12.34"sv);
    EXPECT_EQ(tok_list.front().type, TokenType::Number);
  }
  {
    TokenizerData test_data{"12.34 -56;"sv};
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
    EXPECT_FALSE(tok_result);
    EXPECT_EQ(tok_result.error().type, ParseError::ErrType::Done);
    // NOLINTBEGIN(*unused-return-value*)
    EXPECT_NO_THROW(tok_list.at(0));
    EXPECT_NO_THROW(tok_list.at(1));
    EXPECT_NO_THROW(tok_list.at(2));
    // NOLINTEND(*unused-return-value*)
    EXPECT_EQ(tok_list.at(0).lexeme, "12.34"sv);
    EXPECT_EQ(tok_list.at(1).lexeme, "-"sv);
    EXPECT_EQ(tok_list.at(1).type, TokenType::Minus);
    EXPECT_EQ(tok_list.at(2).lexeme, "56"sv);
    EXPECT_EQ(tok_list.front().type, TokenType::Number);
  }
}

TEST(Tokenizer, KwAndId) {
  {
    TokenizerData test_data{"hEllo;"sv};
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
    EXPECT_FALSE(tok_result);
    EXPECT_EQ(tok_result.error().type, ParseError::ErrType::Done);
    EXPECT_EQ(tok_list.front().lexeme, "hEllo"sv);
    EXPECT_EQ(tok_list.front().type, TokenType::Identifier);
  }
  {
    TokenizerData test_data{"and;"sv};
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
    EXPECT_FALSE(tok_result);
    EXPECT_EQ(tok_result.error().type, ParseError::ErrType::Done);
    EXPECT_EQ(tok_list.front().lexeme, "and"sv);
    EXPECT_EQ(tok_list.front().type, TokenType::And);
  }
}
