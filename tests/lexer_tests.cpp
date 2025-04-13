#include <gtest/gtest.h>
#include "Lexer/Lexer.h"

TEST(LexerTest, TokenizesBasicInstructions) {
  std::string input = "mv r0, r1";
  Lexer lexer(input);
  std::vector<Token> tokens = lexer.tokenize();
  
  ASSERT_EQ(tokens.size(), 5);
  EXPECT_EQ(tokens[0].type, TokenType::INSTRUCTION);
  EXPECT_EQ(tokens[0].value, "mv");
}

TEST(LexerTest, HandlesImmediateValues) {
  std::string input = "add r0, #42";
  Lexer lexer(input);
  std::vector<Token> tokens = lexer.tokenize();
  
  ASSERT_GE(tokens.size(), 5);
  EXPECT_EQ(tokens[3].type, TokenType::NUMBER_IMMEDIATE);
  EXPECT_EQ(tokens[3].value, "42");
}

TEST(LexerTest, TokenizesMemoryInstructions) {
  std::string input = "ld r0, [r1]";
  Lexer lexer(input);
  std::vector<Token> tokens = lexer.tokenize();
  
  ASSERT_EQ(tokens.size(), 7);
  EXPECT_EQ(tokens[0].type, TokenType::INSTRUCTION);
  EXPECT_EQ(tokens[3].type, TokenType::BRACKET_OPEN);
  EXPECT_EQ(tokens[5].type, TokenType::BRACKET_CLOSE);
}

TEST(LexerTest, TokenizesBranchInstructions) {
  std::string input = "beq LOOP";
  Lexer lexer(input);
  std::vector<Token> tokens = lexer.tokenize();
  
  ASSERT_EQ(tokens.size(), 3);
  EXPECT_EQ(tokens[0].type, TokenType::INSTRUCTION);
  EXPECT_EQ(tokens[0].value, "beq");
  EXPECT_EQ(tokens[1].type, TokenType::LABEL_REF);
  EXPECT_EQ(tokens[1].value, "LOOP");
}

TEST(LexerTest, TokenizesXorInstruction) {
  std::string input = "xor r0, r1";
  Lexer lexer(input);
  std::vector<Token> tokens = lexer.tokenize();
  
  ASSERT_EQ(tokens.size(), 5);
  EXPECT_EQ(tokens[0].type, TokenType::INSTRUCTION);
  EXPECT_EQ(tokens[0].value, "xor");
}

TEST(LexerTest, TokenizesShiftInstructions) {
  std::string input = "lsr r1, r2";
  Lexer lexer(input);
  std::vector<Token> tokens = lexer.tokenize();
  
  ASSERT_GE(tokens.size(), 5);
  EXPECT_EQ(tokens[0].type, TokenType::INSTRUCTION);
  EXPECT_EQ(tokens[0].value, "lsr");

  std::string input2 = "lsl r1, #4";
  Lexer lexer2(input2);
  std::vector<Token> tokens2 = lexer2.tokenize();
  
  ASSERT_GE(tokens2.size(), 4);
  EXPECT_EQ(tokens2[0].type, TokenType::INSTRUCTION);
  EXPECT_EQ(tokens2[0].value, "lsl");
}

TEST(LexerTest, TokenizesLabelDefinitions) {
  std::string input = "LOOP: add r0, #1";
  Lexer lexer(input);
  std::vector<Token> tokens = lexer.tokenize();
  
  ASSERT_GE(tokens.size(), 5);
  EXPECT_EQ(tokens[0].type, TokenType::LABEL);
  EXPECT_EQ(tokens[0].value, "LOOP");
}

TEST(LexerTest, TokenizesDirectives) {
  std::string input = ".word 0x1234\n.define MAX_COUNT 100";
  Lexer lexer(input);
  std::vector<Token> tokens = lexer.tokenize();
  
  ASSERT_GE(tokens.size(), 5);
  EXPECT_EQ(tokens[0].type, TokenType::DIRECTIVE);
  EXPECT_EQ(tokens[0].value, ".word");
  EXPECT_EQ(tokens[2].type, TokenType::DIRECTIVE);
  EXPECT_EQ(tokens[2].value, ".define");
}