#include <gtest/gtest.h>
#include "Parser/Parser.h"
#include "Lexer/Lexer.h"

TEST(ParserTest, ParsesMovInstruction) {
    std::string input = "mv r0, r1";
    Lexer lexer(input);
    std::vector<Token> tokens = lexer.tokenize();
    
    Parser parser(tokens);
    auto statements = parser.parse();
    
    ASSERT_EQ(statements.size(), 1);
    ASSERT_EQ(statements[0]->type, StatementType::INSTRUCTION);
    
    auto* instr = static_cast<Instruction*>(statements[0].get());
    EXPECT_EQ(instr->opcode, "mv");
    EXPECT_EQ(instr->operand1, "r0");
    EXPECT_EQ(instr->operand2, "r1");
    EXPECT_TRUE(instr->hasComma);
    EXPECT_FALSE(instr->isImmediate);
}

TEST(ParserTest, HandlesLabel) {
    std::string input = "LOOP: add r0, #1";
    Lexer lexer(input);
    std::vector<Token> tokens = lexer.tokenize();
    
    Parser parser(tokens);
    auto statements = parser.parse();
    
    ASSERT_EQ(statements.size(), 2);
    ASSERT_EQ(statements[0]->type, StatementType::LABEL);
    
    auto* label = static_cast<Label*>(statements[0].get());
    EXPECT_EQ(label->name, "LOOP");
}