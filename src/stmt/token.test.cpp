#include <gtest/gtest.h>
#ifndef TINYDB_IMPORT_STD
#include <array>
#include <ranges>
#include <string_view>
#endif
#ifdef TINYDB_MODULE
import tinydb.stmt;
#ifdef TINYDB_IMPORT_STD
import std;
#endif
#else
#include "token.hpp"
#endif

using namespace std::literals;
using namespace tinydb::stmt;

TEST(Tokenizer, Empty) {
  auto tok_result = tokenize(""sv);
  EXPECT_FALSE(tok_result);
  EXPECT_EQ(tok_result.error().type, ParseError::ErrType::UnendedStmt);
}

TEST(Tokenizer, Num) {

  {
    auto tok_result = tokenize("12.34;"sv);
    EXPECT_TRUE(tok_result);
    auto &tok_list = *tok_result;
    EXPECT_FALSE(tok_list.empty());
    EXPECT_EQ(tok_list.back().lexeme, ";"sv);
    EXPECT_EQ(tok_list.back().type, TokenType::Semicolon);
    EXPECT_EQ(tok_list.front().lexeme, "12.34"sv);
    EXPECT_EQ(tok_list.front().type, TokenType::Number);
  }
  {
    auto tok_result = tokenize("12.34 -56;"sv);
    EXPECT_TRUE(tok_result);
    std::array expected_lexemes{"12.34"sv, "-"sv, "56"sv, ";"sv};
    using enum TokenType;
    std::array expected_types{Number, Minus, Number, Semicolon};
    auto &tok_list = *tok_result;
    for (auto [tok, ex_lex, ex_typ] :
         std::views::zip(tok_list, expected_lexemes, expected_types)) {
      EXPECT_EQ(tok.lexeme, ex_lex);
      EXPECT_EQ(tok.type, ex_typ);
    }
    EXPECT_FALSE(tok_list.empty());
  }
}

TEST(Tokenizer, KwAndId) {
  {
    auto tok_result = tokenize("hEllo;"sv);
    EXPECT_TRUE(tok_result);
    auto &tok_list = *tok_result;
    EXPECT_FALSE(tok_list.empty());
    // NOLINTBEGIN(*unused-return-value*)
    EXPECT_NO_THROW(tok_list.at(0));
    EXPECT_NO_THROW(tok_list.at(1));
    // NOLINTEND(*unused-return-value*)
    EXPECT_EQ(tok_list.at(0).lexeme, "hEllo"sv);
    EXPECT_EQ(tok_list.at(0).type, TokenType::Identifier);
    EXPECT_EQ(tok_list.at(1).lexeme, ";"sv);
    EXPECT_EQ(tok_list.at(1).type, TokenType::Semicolon);
  }
  {
    auto tok_result = tokenize("and;"sv);
    EXPECT_TRUE(tok_result);
    auto &tok_list = *tok_result;
    EXPECT_FALSE(tok_list.empty());
    std::array expected_lexemes{"and"sv, ";"sv};
    using enum TokenType;
    std::array expected_types{TokenType::And, TokenType::Semicolon};
    EXPECT_EQ(tok_list.size(), expected_lexemes.size());
    EXPECT_EQ(tok_list.size(), expected_types.size());
    for (auto [tok, exp_lex, exp_typ] :
         std::views::zip(tok_list, expected_lexemes, expected_types)) {
      EXPECT_EQ(tok.lexeme, exp_lex);
      EXPECT_EQ(tok.type, exp_typ);
    }
  }
  {
    auto tok_result = tokenize("select   col from HiEr;");
    EXPECT_TRUE(tok_result);
    auto &tok_list = *tok_result;
    EXPECT_FALSE(tok_list.empty());
    std::array expected_lexemes{"select"sv, "col"sv, "from"sv, "HiEr"sv, ";"sv};
    using enum TokenType;
    std::array expected_types{Select, Identifier, From, Identifier, Semicolon};
    EXPECT_EQ(tok_list.size(), expected_lexemes.size());
    EXPECT_EQ(tok_list.size(), expected_types.size());
    for (auto [tok, exp_lex, exp_typ] :
         std::views::zip(tok_list, expected_lexemes, expected_types)) {
           EXPECT_EQ(tok.lexeme, exp_lex);
           EXPECT_EQ(tok.type, exp_typ);
    }
  }
}
