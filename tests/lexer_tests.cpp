#include <gtest/gtest.h>
#include "Lexer/Lexer.h"

TEST(LexerTest, TokenizesInstruction) {
  std::string input = "mv r0, r1";
  Lexer lexer(input);
  std::vector<Token> tokens = lexer.tokenize();
  
  ASSERT_EQ(tokens.size(), 5);
  
  EXPECT_EQ(tokens[0].type, TokenType::INSTRUCTION);
  EXPECT_EQ(tokens[0].value, "mv");
  
  EXPECT_EQ(tokens[1].type, TokenType::REGISTER);
  EXPECT_EQ(tokens[1].value, "r0");
  
  EXPECT_EQ(tokens[2].type, TokenType::COMMA);
  
  EXPECT_EQ(tokens[3].type, TokenType::REGISTER);
  EXPECT_EQ(tokens[3].value, "r1");
  
  EXPECT_EQ(tokens[4].type, TokenType::END_OF_FILE);
}

TEST(LexerTest, HandlesImmediate) {
  std::string input = "add r0, #42";
  Lexer lexer(input);
  std::vector<Token> tokens = lexer.tokenize();
  
  ASSERT_GE(tokens.size(), 5);
  EXPECT_EQ(tokens[3].type, TokenType::NUMBER_IMMEDIATE);
  EXPECT_EQ(tokens[3].value, "42");
}