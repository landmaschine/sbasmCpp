#include <gtest/gtest.h>
#include "Parser/Parser.h"
#include "Lexer/Lexer.h"

std::vector<std::unique_ptr<Statement>> parseInput(const std::string& input) {
    Lexer lexer(input);
    auto tokens = lexer.tokenize();
    Parser parser(tokens);
    return parser.parse();
}

TEST(ParserTest, ParsesRegisterToRegisterMove) {
    auto statements = parseInput("mv r0, r1");
    
    ASSERT_EQ(statements.size(), 1);
    ASSERT_EQ(statements[0]->type, StatementType::INSTRUCTION);
    
    auto* instr = static_cast<Instruction*>(statements[0].get());
    EXPECT_EQ(instr->opcode, "mv");
    EXPECT_EQ(instr->operand1, "r0");
    EXPECT_EQ(instr->operand2, "r1");
    EXPECT_FALSE(instr->isImmediate);
}

TEST(ParserTest, ParsesImmediateMove) {
    auto statements = parseInput("mv r0, #42");
    
    ASSERT_EQ(statements.size(), 1);
    auto* instr = static_cast<Instruction*>(statements[0].get());
    EXPECT_EQ(instr->opcode, "mv");
    EXPECT_TRUE(instr->isImmediate);
    EXPECT_EQ(instr->operand2, "42");
}

TEST(ParserTest, ParsesLabelImmediateMove) {
    auto statements = parseInput("mv r0, =1234");
    
    ASSERT_EQ(statements.size(), 1);
    auto* instr = static_cast<Instruction*>(statements[0].get());
    EXPECT_EQ(instr->opcode, "mv");
    EXPECT_TRUE(instr->isLabelImmediate);
    EXPECT_EQ(instr->operand2, "1234");
}

TEST(ParserTest, ParsesUnconditionalBranch) {
    auto statements = parseInput("b LOOP");
    
    ASSERT_EQ(statements.size(), 1);
    auto* instr = static_cast<Instruction*>(statements[0].get());
    EXPECT_EQ(instr->opcode, "b");
    EXPECT_EQ(instr->operand1, "LOOP");
}

TEST(ParserTest, ParsesConditionalBranch) {
    auto statements = parseInput("beq TARGET");
    
    ASSERT_EQ(statements.size(), 1);
    auto* instr = static_cast<Instruction*>(statements[0].get());
    EXPECT_EQ(instr->opcode, "beq");
    EXPECT_EQ(instr->operand1, "TARGET");
}

TEST(ParserTest, ParsesMoveTop) {
    auto statements = parseInput("mvt r0, #255");
    
    ASSERT_EQ(statements.size(), 1);
    auto* instr = static_cast<Instruction*>(statements[0].get());
    EXPECT_EQ(instr->opcode, "mvt");
    EXPECT_TRUE(instr->isImmediate);
}

TEST(ParserTest, ParsesALURegisterOps) {
    auto statements = parseInput("add r0, r1\nsub r2, r3\nand r4, r5\nxor r6, r7");
    
    ASSERT_EQ(statements.size(), 4);
    
    auto* add = static_cast<Instruction*>(statements[0].get());
    EXPECT_EQ(add->opcode, "add");
    EXPECT_FALSE(add->isImmediate);
    
    auto* sub = static_cast<Instruction*>(statements[1].get());
    EXPECT_EQ(sub->opcode, "sub");
    
    auto* and_op = static_cast<Instruction*>(statements[2].get());
    EXPECT_EQ(and_op->opcode, "and");
    
    auto* xor_op = static_cast<Instruction*>(statements[3].get());
    EXPECT_EQ(xor_op->opcode, "xor");
}

TEST(ParserTest, ParsesALUImmediateOps) {
    auto statements = parseInput("add r0, #10\nsub r2, #20\nand r4, #30");
    
    ASSERT_EQ(statements.size(), 3);
    
    for (const auto& stmt : statements) {
        auto* instr = static_cast<Instruction*>(stmt.get());
        EXPECT_TRUE(instr->isImmediate);
    }
}

TEST(ParserTest, ParsesMemoryOps) {
    auto statements = parseInput("ld r0, [r1]\nst r2, [r3]");
    
    ASSERT_EQ(statements.size(), 2);
    
    auto* load = static_cast<Instruction*>(statements[0].get());
    EXPECT_EQ(load->opcode, "ld");
    EXPECT_EQ(load->operand1, "r0");
    EXPECT_EQ(load->operand2, "r1");
    
    auto* store = static_cast<Instruction*>(statements[1].get());
    EXPECT_EQ(store->opcode, "st");
}

TEST(ParserTest, ParsesStackOps) {
    auto statements = parseInput("push r0\npop r1");
    
    ASSERT_EQ(statements.size(), 2);
    
    auto* push = static_cast<Instruction*>(statements[0].get());
    EXPECT_EQ(push->opcode, "push");
    EXPECT_EQ(push->operand1, "r0");
    
    auto* pop = static_cast<Instruction*>(statements[1].get());
    EXPECT_EQ(pop->opcode, "pop");
}

TEST(ParserTest, ParsesCompareOps) {
    auto statements = parseInput("cmp r0, r1\ncmp r2, #42");
    
    ASSERT_EQ(statements.size(), 2);
    
    auto* cmp_reg = static_cast<Instruction*>(statements[0].get());
    EXPECT_EQ(cmp_reg->opcode, "cmp");
    EXPECT_FALSE(cmp_reg->isImmediate);
    
    auto* cmp_imm = static_cast<Instruction*>(statements[1].get());
    EXPECT_EQ(cmp_imm->opcode, "cmp");
    EXPECT_TRUE(cmp_imm->isImmediate);
}

TEST(ParserTest, ParsesShiftOps) {
    auto statements = parseInput("lsl r0, r1\nlsr r2, #2\nasr r3, r4\nror r5, #3");
    
    ASSERT_EQ(statements.size(), 4);
    
    auto* lsl = static_cast<Instruction*>(statements[0].get());
    EXPECT_EQ(lsl->opcode, "lsl");
    EXPECT_FALSE(lsl->isImmediate);
    
    auto* lsr = static_cast<Instruction*>(statements[1].get());
    EXPECT_EQ(lsr->opcode, "lsr");
    EXPECT_TRUE(lsr->isImmediate);
    
    auto* asr = static_cast<Instruction*>(statements[2].get());
    EXPECT_EQ(asr->opcode, "asr");
    
    auto* ror = static_cast<Instruction*>(statements[3].get());
    EXPECT_EQ(ror->opcode, "ror");
}

TEST(ParserTest, ParsesDirectives) {
    auto statements = parseInput(".define MAX 100\n.word 0xABCD");
    
    ASSERT_EQ(statements.size(), 2);
    ASSERT_EQ(statements[0]->type, StatementType::DIRECTIVE);
    ASSERT_EQ(statements[1]->type, StatementType::DIRECTIVE);
    
    auto* define = static_cast<Directive*>(statements[0].get());
    EXPECT_EQ(define->name, ".define");
    EXPECT_EQ(define->label, "MAX");
    EXPECT_EQ(define->value, "100");
    
    auto* word = static_cast<Directive*>(statements[1].get());
    EXPECT_EQ(word->name, ".word");
    EXPECT_EQ(word->value, "0xABCD");
}