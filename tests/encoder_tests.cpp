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
    symbolTable.addLabel("LOOP", 0x50);
  }
  
  uint16_t encodeSingleInstruction(const std::string& opcode, 
                                  const std::string& op1, 
                                  const std::string& op2 = "",
                                  bool hasComma = true,
                                  bool isLabelImm = false,
                                  bool isImm = false) {
    std::vector<std::unique_ptr<Statement>> ast;
    ast.push_back(std::make_unique<Instruction>(
      opcode, op1, op2, hasComma, isLabelImm, isImm, 1, 1
    ));
    
    auto result = encoder.encode(ast);
    EXPECT_EQ(result.size(), 1);
    return result[0];
  }
};

TEST_F(EncoderTest, EncodesMoveRegister) {
  EXPECT_EQ(encodeSingleInstruction("mv", "r0", "r1"), 0x0001);
  EXPECT_EQ(encodeSingleInstruction("mv", "r3", "r7"), 0x0607);
}

TEST_F(EncoderTest, EncodesMoveImmediate) {
  EXPECT_EQ(encodeSingleInstruction("mv", "r1", "42", true, false, true), 0x122A);
  EXPECT_EQ(encodeSingleInstruction("mv", "r5", "255", true, false, true), 0x1AFF);
}

TEST_F(EncoderTest, EncodesUnconditionalBranch) {
  EXPECT_EQ(encodeSingleInstruction("b", "LOOP"), 0x204F);
}

TEST_F(EncoderTest, EncodesConditionalBranches) {
  EXPECT_EQ(encodeSingleInstruction("beq", "LOOP"), 0x224F);
  EXPECT_EQ(encodeSingleInstruction("bne", "LOOP"), 0x244F);
  EXPECT_EQ(encodeSingleInstruction("bcc", "LOOP"), 0x264F);
  EXPECT_EQ(encodeSingleInstruction("bcs", "LOOP"), 0x284F);
  EXPECT_EQ(encodeSingleInstruction("bpl", "LOOP"), 0x2A4F);
  EXPECT_EQ(encodeSingleInstruction("bmi", "LOOP"), 0x2C4F);
  EXPECT_EQ(encodeSingleInstruction("bl", "LOOP"), 0x2E4F);
}

TEST_F(EncoderTest, EncodesMoveTop) {
  EXPECT_EQ(encodeSingleInstruction("mvt", "r0", "0x12", true, false, true), 0x3012);
  EXPECT_EQ(encodeSingleInstruction("mvt", "r7", "0xFF", true, false, true), 0x3EFF);
}

TEST_F(EncoderTest, EncodesALURegisterOps) {
  EXPECT_EQ(encodeSingleInstruction("add", "r0", "r1"), 0x4001);
  EXPECT_EQ(encodeSingleInstruction("sub", "r2", "r3"), 0x6403);
  EXPECT_EQ(encodeSingleInstruction("and", "r4", "r5"), 0xC805);
  EXPECT_EQ(encodeSingleInstruction("xor", "r6", "r7"), 0xED17);
}

TEST_F(EncoderTest, EncodesALUImmediateOps) {
  EXPECT_EQ(encodeSingleInstruction("add", "r0", "42", true, false, true), 0x502A);
  EXPECT_EQ(encodeSingleInstruction("sub", "r3", "100", true, false, true), 0x7664);
  EXPECT_EQ(encodeSingleInstruction("and", "r5", "0xFF", true, false, true), 0xDAFF);
}

TEST_F(EncoderTest, EncodesMemoryOps) {
  EXPECT_EQ(encodeSingleInstruction("ld", "r0", "r1"), 0x8001);
  EXPECT_EQ(encodeSingleInstruction("st", "r2", "r3"), 0xA403);
}

TEST_F(EncoderTest, EncodesStackOps) {
  EXPECT_EQ(encodeSingleInstruction("push", "r0"), 0xB005);
  EXPECT_EQ(encodeSingleInstruction("pop", "r7"), 0x9E05);
}

TEST_F(EncoderTest, EncodesCompareOps) {
  EXPECT_EQ(encodeSingleInstruction("cmp", "r0", "r1"), 0xE001);
  EXPECT_EQ(encodeSingleInstruction("cmp", "r7", "64", true, false, true), 0xFE40);
}

TEST_F(EncoderTest, EncodesShiftOps) {
  EXPECT_EQ(encodeSingleInstruction("lsl", "r0", "r1"), 0xE101);
  EXPECT_EQ(encodeSingleInstruction("lsl", "r2", "4", true, false, true), 0xE584);
  
  EXPECT_EQ(encodeSingleInstruction("lsr", "r3", "r4"), 0xE724);
  EXPECT_EQ(encodeSingleInstruction("lsr", "r5", "8", true, false, true), 0xEBA8);
  
  EXPECT_EQ(encodeSingleInstruction("asr", "r6", "r7"), 0xED47);
  EXPECT_EQ(encodeSingleInstruction("asr", "r0", "2", true, false, true), 0xE1C2);
  
  EXPECT_EQ(encodeSingleInstruction("ror", "r1", "r2"), 0xE362);
  EXPECT_EQ(encodeSingleInstruction("ror", "r3", "6", true, false, true), 0xE7E6);
}

TEST_F(EncoderTest, HandlesMultiInstructionSequence) {
  std::vector<std::unique_ptr<Statement>> ast;
  
  auto instr1 = std::make_unique<Instruction>("mv", "r0", "0x1234", true, true, true, 1, 1);
  ast.push_back(std::move(instr1));
  
  auto result = encoder.encode(ast);
  
  ASSERT_EQ(result.size(), 2);
  EXPECT_EQ(result[0], 0x3012);
  EXPECT_EQ(result[1], 0x5034);
}

TEST_F(EncoderTest, HandlesWordDirective) {
  std::vector<std::unique_ptr<Statement>> ast;
  
  auto dir = std::make_unique<Directive>(".word", "", "0xABCD", 1, 1);
  ast.push_back(std::move(dir));
  
  auto result = encoder.encode(ast);
  
  ASSERT_EQ(result.size(), 1);
  EXPECT_EQ(result[0], 0xABCD);
}