#include <gtest/gtest.h>
#include "InstructionEncoder/InstructionEncoder.h"
#include "InstructionEncoder/SymbolTable.h"
#include "Parser/Parser.h"

class EncoderTest : public ::testing::Test {
protected:
  SymbolTable symbolTable;
  Encoder encoder{symbolTable};
    
  void SetUp() override {
    symbolTable.addDefine("TEST_VALUE", 42);
    symbolTable.addLabel("test_label", 0x100);
  }
};

TEST_F(EncoderTest, EncodesMoveRegister) {
  std::vector<std::unique_ptr<Statement>> ast;
  
  auto instr = std::make_unique<Instruction>("mv", "r0", "r1", true, false, false, 1, 1);
  ast.push_back(std::move(instr));
  
  auto result = encoder.encode(ast);
  
  ASSERT_EQ(result.size(), 1);
  EXPECT_EQ(result[0], 0x0001);
}

TEST_F(EncoderTest, EncodesImmediateValue) {
  std::vector<std::unique_ptr<Statement>> ast;
  
  auto instr = std::make_unique<Instruction>("mv", "r1", "42", true, false, true, 1, 1);
  ast.push_back(std::move(instr));
  
  auto result = encoder.encode(ast);
  
  ASSERT_EQ(result.size(), 1);
  EXPECT_EQ(result[0], 0x122A);
}