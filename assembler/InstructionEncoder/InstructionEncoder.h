#pragma once
#include <unordered_map>
#include "SymbolTable.h"
#include <vector>
#include "Parser/Parser.h"

class Encoder {
private:
    SymbolTable& symbolTable;
    std::vector<uint16_t> machineCode;
    int currentAddress;

    static constexpr uint16_t MV_REG   = 0x0000;
    static constexpr uint16_t MV_IMM   = 0x1000;
    static constexpr uint16_t BRANCH   = 0x2000;
    static constexpr uint16_t MVT      = 0x3000;
    static constexpr uint16_t ADD_REG  = 0x4000;
    static constexpr uint16_t ADD_IMM  = 0x5000;
    static constexpr uint16_t SUB_REG  = 0x6000;
    static constexpr uint16_t SUB_IMM  = 0x7000;
    static constexpr uint16_t LD       = 0x8000;
    static constexpr uint16_t POP      = 0x9000;
    static constexpr uint16_t ST       = 0xA000;
    static constexpr uint16_t PUSH     = 0xB000;
    static constexpr uint16_t AND_REG  = 0xC000;
    static constexpr uint16_t AND_IMM  = 0xD000;
    static constexpr uint16_t CMP_REG  = 0xE000;
    static constexpr uint16_t CMP_IMM  = 0xF000;

    uint8_t parseRegister(const std::string& reg);

    uint8_t parseBranchCond(const std::string& opcode);

    uint8_t parseShiftType(const std::string& op);

    uint16_t encodeImmediate(int64_t value, int bits, const std::string& context);

    int64_t parseImmediateOrSymbol(const std::string& value, const std::string& context);

    void encodeDirective(Directive* dir);
    void encodeMoveInstruction(Instruction* instr, const uint8_t rX);

    void encodeBranchInstruction(Instruction* instr);

    void encodeALUInstruction(Instruction* instr, const uint8_t rX);

    void encodeMemoryInstruction(Instruction* instr, const uint8_t rX);

    void encodeCompareInstruction(Instruction* instr, const uint8_t rX);
    void encodeShiftInstruction(Instruction* instr, const uint8_t rX);

    void encodeMovTopInstruction(Instruction* instr, const uint8_t rX);
    void encodeInstruction(Instruction* instr);

public:
    Encoder(SymbolTable& st) : symbolTable(st), currentAddress(0) {}

    std::vector<uint16_t> encode(const std::vector<std::unique_ptr<Statement>>& ast);
};