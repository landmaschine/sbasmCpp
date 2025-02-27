// ----------------------------------------------------------------------------
// Author: LeonW
// Date: February 3, 2025
// ----------------------------------------------------------------------------

#include "InstructionEncoder.h"

uint8_t Encoder::parseRegister(const std::string& reg) {
    static const std::unordered_map<std::string, uint8_t> regMap = {
        {"r0", 0}, {"r1", 1}, {"r2", 2}, {"r3", 3},
        {"r4", 4}, {"r5", 5}, {"r6", 6}, {"r7", 7},
        {"sp", 5}, {"lr", 6}, {"pc", 7}
    };
    
    auto it = regMap.find(reg);
    if(it == regMap.end()) {
        throw std::runtime_error("Invalid register name: " + reg);
    }
    return it->second;
}

uint8_t Encoder::parseBranchCond(const std::string& opcode) {
    static const std::unordered_map<std::string, uint8_t> condMap = {
        {"b", 0}, {"beq", 1}, {"bne", 2}, {"bcc", 3},
        {"bcs", 4}, {"bpl", 5}, {"bmi", 6}, {"bl", 7}
    };
    
    auto it = condMap.find(opcode);
    if(it == condMap.end()) {
        throw std::runtime_error("Invalid branch condition: " + opcode);
    }
    return it->second;
}

uint8_t Encoder::parseShiftType(const std::string& op) {
    static const std::unordered_map<std::string, uint8_t> shiftMap = {
        {"lsl", 0}, {"lsr", 1}, {"asr", 2}, {"ror", 3}
    };
    
    auto it = shiftMap.find(op);
    if(it == shiftMap.end()) {
        throw std::runtime_error("Invalid shift type: " + op);
    }
    return it->second;
}

uint16_t Encoder::encodeImmediate(int64_t value, int bits, const std::string& context) {
    int64_t maxVal = (1ll << (bits - 1)) - 1;
    int64_t minVal = -(1ll << (bits - 1));
    
    if (value > maxVal || value < minVal) {
        throw std::runtime_error("Immediate value " + std::to_string(value) + 
                                " out of range [" + std::to_string(minVal) + ", " + 
                                std::to_string(maxVal) + "] for " + context);
    }
    
    if (value < 0) 
        value += (1 << bits);
    return static_cast<uint16_t>(value & ((1 << bits) - 1));
}

int64_t Encoder::parseImmediateOrSymbol(const std::string& value, const std::string& context) {
    try {
        if (symbolTable.hasDefine(value)) {
            return symbolTable.getDefineValue(value);
        }
        
        std::string numStr = value;
        if (numStr[0] == '#') numStr = numStr.substr(1);
        
        if (numStr.size() >= 2) {
            if (numStr.substr(0, 2) == "0x" || numStr.substr(0, 2) == "0X") {
                return std::stoll(numStr.substr(2), nullptr, 16);
            }
            if (numStr.substr(0, 2) == "0b" || numStr.substr(0, 2) == "0B") {
                return std::stoll(numStr.substr(2), nullptr, 2);
            }
        }
        return std::stoll(numStr, nullptr, 0);
    } catch (const std::exception& e) {
        throw std::runtime_error("Failed to parse immediate value '" + value + "' for " + context + ": " + e.what());
    }
}

void Encoder::encodeDirective(Directive* dir) {
    try {
        if (dir->name == ".word") {
            int64_t value = parseImmediateOrSymbol(dir->value, ".word directive");
            if (value > 0xFFFF || value < -0x8000) {
                throw std::runtime_error(".word value out of range [-32768, 65535]");
            }
            machineCode.push_back(static_cast<uint16_t>(value & 0xFFFF));
            currentAddress++;
        }
    } catch (const std::exception& e) {
        throw std::runtime_error("Error encoding directive at line " + 
                                std::to_string(dir->line) + ": " + e.what());
    }
}

void Encoder::encodeMoveInstruction(Instruction* instr, const uint8_t rX) {
    if (instr->isLabelImmediate) {
        int64_t value;
        if (symbolTable.hasLabel(instr->operand2)) {
            value = symbolTable.getLabelAddress(instr->operand2);
        } else if (symbolTable.hasDefine(instr->operand2)) {
            value = symbolTable.getDefineValue(instr->operand2);
        } else {
            value = parseImmediateOrSymbol(instr->operand2, "move label immediate");
        }
        
        machineCode.push_back(MVT | (rX << 9) | ((value >> 8) & 0xFF));
        machineCode.push_back(ADD_IMM | (rX << 9) | (value & 0xFF));
        currentAddress += 2;
        return;
    }

    if (instr->isImmediate) {
        int64_t value;
        if (!symbolTable.hasDefine(instr->operand2)) {
            value = parseImmediateOrSymbol(instr->operand2, "move immediate");
            if (value > 255 || value < -256) {
                throw std::runtime_error("Immediate value with # must fit in 9 bits (-256 to 255), got: " + std::to_string(value) + ". Use = for larger values.");
            }
            machineCode.push_back(MV_IMM | (rX << 9) | encodeImmediate(value, 9, "move"));
            currentAddress++;
            return;
        } else {
            value = symbolTable.getDefineValue(instr->operand2);
            if (value > 255 || value < -256) {
                throw std::runtime_error("Defined symbol value must fit in 9 bits when used with #. Symbol: " + instr->operand2 + ", Value: " + std::to_string(value));
            }
            machineCode.push_back(MV_IMM | (rX << 9) | encodeImmediate(value, 9, "move"));
            currentAddress++;
            return;
        }
    }

    const uint8_t rY = parseRegister(instr->operand2);
    machineCode.push_back(MV_REG | (rX << 9) | rY);
    currentAddress++;
}

void Encoder::encodeBranchInstruction(Instruction* instr) {
    const uint8_t condition = parseBranchCond(instr->opcode);
    const int targetAddr = symbolTable.getLabelAddress(instr->operand1);
    const int offset = targetAddr - (currentAddress + 1);
    
    if (offset > 255 || offset < -256) {
        throw std::runtime_error("Branch target too far (offset " + std::to_string(offset) + " words)");
    }
    
    machineCode.push_back(BRANCH | (condition << 9) | encodeImmediate(offset, 9, "branch offset"));
    currentAddress++;
}

void Encoder::encodeALUInstruction(Instruction* instr, const uint8_t rX) {
    uint16_t baseOpcode;
    std::string context;
    
    if (instr->opcode == "add") {
        baseOpcode = instr->isImmediate ? ADD_IMM : ADD_REG;
        context = "add";
    } else if (instr->opcode == "sub") {
        baseOpcode = instr->isImmediate ? SUB_IMM : SUB_REG;
        context = "subtract";
    } else if (instr->opcode == "and") {
        baseOpcode = instr->isImmediate ? AND_IMM : AND_REG;
        context = "and";
    } else if(instr->opcode == "xor") {
        baseOpcode = XOR_REG;
        instr->isImmediate = false;
        context = "xor";
    } else {
        throw std::runtime_error("Unknown ALU instruction: " + instr->opcode);
    }

    if (instr->isImmediate) {
        const int64_t imm = parseImmediateOrSymbol(instr->operand2, context);
        if (instr->isLabelImmediate) {
            if (imm > 0xFFFF || imm < -0x8000) {
                throw std::runtime_error("16-bit immediate value out of range (-32768 to 65535)");
            }
            machineCode.push_back(MVT | (rX << 9) | ((imm >> 8) & 0xFF));
            machineCode.push_back(baseOpcode | (rX << 9) | (imm & 0xFF));
            currentAddress += 2;
        } else {
            if (imm > 255 || imm < -256) {
                throw std::runtime_error("Immediate value with # must fit in 9 bits (-256 to 255), got: " + std::to_string(imm) + ". Use = for larger values.");
            }
            machineCode.push_back(baseOpcode | (rX << 9) | encodeImmediate(imm, 9, context));
            currentAddress++;
        }
    } else {
        const uint8_t rY = parseRegister(instr->operand2);
        machineCode.push_back(baseOpcode | (rX << 9) | rY);
        currentAddress++;
    }
} 

void Encoder::encodeMemoryInstruction(Instruction* instr, const uint8_t rX) {
    if (instr->opcode == "ld") {
        const uint8_t rY = parseRegister(instr->operand2);
        machineCode.push_back(LD | (rX << 9) | rY);
    } else if (instr->opcode == "st") {
        const uint8_t rY = parseRegister(instr->operand2);
        machineCode.push_back(ST | (rX << 9) | rY);
    } else if (instr->opcode == "pop") {
        machineCode.push_back(POP | (rX << 9) | 0x05);
    } else if (instr->opcode == "push") {
        machineCode.push_back(PUSH | (rX << 9) | 0x05);
    }
    currentAddress++;
}

void Encoder::encodeCompareInstruction(Instruction* instr, const uint8_t rX) {
    if (instr->isImmediate) {
        const int64_t imm = parseImmediateOrSymbol(instr->operand2, "compare");
        machineCode.push_back(CMP_IMM | (rX << 9) | encodeImmediate(imm, 9, "compare"));
    } else {
        const uint8_t rY = parseRegister(instr->operand2);
        machineCode.push_back(CMP_REG | (rX << 9) | rY);
    }
    currentAddress++;
}

void Encoder::encodeShiftInstruction(Instruction* instr, const uint8_t rX) {
    const uint8_t shiftType = parseShiftType(instr->opcode);
    uint16_t encoded = CMP_REG | (rX << 9) | (0b10 << 7) | (shiftType << 5);
    
    if (instr->isImmediate) {
        const int64_t imm = parseImmediateOrSymbol(instr->operand2, "shift amount");
        if (imm > 15 || imm < 0) {
            throw std::runtime_error("Shift amount must be between 0 and 15");
        }
        encoded |= (1 << 7) | (imm & 0xF);
    } else {
        const uint8_t rY = parseRegister(instr->operand2);
        encoded |= rY;
    }
    
    machineCode.push_back(encoded);
    currentAddress++;
}

void Encoder::encodeMovTopInstruction(Instruction* instr, const uint8_t rX) {
    const int64_t imm = parseImmediateOrSymbol(instr->operand2, "mvt");
    if (imm > 255 || imm < -128) {
        throw std::runtime_error("MVT immediate value must fit in 8 bits");
    }
    machineCode.push_back(MVT | (rX << 9) | (imm & 0xFF));
    currentAddress++;
}

void Encoder::encodeInstruction(Instruction* instr) {
    try {
        uint8_t rX = 0;
        if (instr->opcode[0] != 'b') {
            rX = parseRegister(instr->operand1);
        }

        if (instr->opcode == "mv") {
            encodeMoveInstruction(instr, rX);
        }
        else if (instr->opcode[0] == 'b') {
            encodeBranchInstruction(instr);
        }
        else if (instr->opcode == "mvt") {
            encodeMovTopInstruction(instr, rX);
        }
        else if (instr->opcode == "add" || instr->opcode == "sub" || instr->opcode == "and" || instr->opcode == "xor") {
            encodeALUInstruction(instr, rX);
        }
        else if (instr->opcode == "ld" || instr->opcode == "st" || instr->opcode == "pop" || instr->opcode == "push") {
            encodeMemoryInstruction(instr, rX);
        }
        else if (instr->opcode == "cmp") {
            encodeCompareInstruction(instr, rX);
        }
        else if (instr->opcode == "lsl" || instr->opcode == "lsr" || instr->opcode == "asr" || instr->opcode == "ror") {
            encodeShiftInstruction(instr, rX);
        }
        else {
            throw std::runtime_error("Unknown instruction: " + instr->opcode);
        }
    } catch (const std::exception& e) {
        throw std::runtime_error("Error encoding instruction at line " + std::to_string(instr->line) + ": " + e.what());
    }
}


std::vector<uint16_t> Encoder::encode(const std::vector<std::unique_ptr<Statement>>& ast) {
    machineCode.clear();
    currentAddress = 0;
    
    for (const auto& stmt : ast) {
        switch (stmt->type) {
            case StatementType::LABEL:
                break;
            case StatementType::DIRECTIVE:
                encodeDirective(static_cast<Directive*>(stmt.get()));
                break;
            case StatementType::INSTRUCTION:
                encodeInstruction(static_cast<Instruction*>(stmt.get()));
                break;
            default:
                throw std::runtime_error("Unknown statement type at line " + std::to_string(stmt->line));
        }
    }
    return machineCode;
}