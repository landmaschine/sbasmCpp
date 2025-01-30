#include "common.h"
#include "Lexer/Lexer.h"
#include "Parser/Parser.h"
#include "InstructionEncoder/InstructionEncoder.h"
#include "InstructionEncoder/SymbolTable.h"
#include <fstream>
#include <sstream>
#include <string>
#include <iomanip>

int main(int argc, const char* argv[]) {
    /*
    if(argc < 2) {
        std::cerr << "Usage: " << argv[0] << " <filename>" << std::endl;
    }

    std::ifstream file(argv[1]);
    if(!file.is_open()) {
        std::cerr << "Error: Could no open file '" << argv[1] << "'" << std::endl;
    }
    */

    std::string asmCode = R"(
        .define LED_ADDRESS 0x1000
        .define HEX_ADDRESS 0x2000
        .define SWI_ADDRESS 0x3000

        START: mv r1, =DATA
            mv r2, =0x3
            add r2, r1
            sub r2, r1

        DATA:  .word 0xDD
            .word 0xFF
    )";

    /*
    std::stringstream buffer;
    buffer << file.rdbuf();
    std::string input = buffer.str();
    file.close();
    */

    Lexer lexer(asmCode);
    std::vector<Token> tokens = lexer.tokenize();
    std::cout << "Tokens:\n";
    for(const auto& token : tokens) {
        std::cout << "Type: " << static_cast<int>(token.type) 
                  << ", Value: " << token.value 
                  << ", Line: " << token.line 
                  << ", Column: " << token.column << std::endl;
    }

    Parser parser(tokens);
    std::vector<std::unique_ptr<Statement>> ast = parser.parse();
    std::cout << "\nAST:\n";
    for(const auto& stmt : ast) {
        switch(stmt->type) {
            case StatementType::LABEL:
                std::cout << "Label: " << static_cast<Label*>(stmt.get())->name << std::endl;
                break;
            case StatementType::DIRECTIVE:
                std::cout << "Directive: " << static_cast<Directive*>(stmt.get())->name 
                         << " " << static_cast<Directive*>(stmt.get())->label 
                         << " " << static_cast<Directive*>(stmt.get())->value << std::endl;
                break;
            case StatementType::INSTRUCTION:
                std::cout << "Instruction: " << static_cast<Instruction*>(stmt.get())->opcode 
                         << " " << static_cast<Instruction*>(stmt.get())->operand1
                         << " " << static_cast<Instruction*>(stmt.get())->operand2 << std::endl;
                break;
        }
    }

    InstructionEncoder encoder;
    SymbolTable symbolTable;
    std::vector<uint16_t> machineCode;
    int currentAddress = 0;

    std::cout << "\nFirst Pass:\n";
    for(const auto& stmt : ast) {
        switch(stmt->type) {
            case StatementType::LABEL: {
                auto label = static_cast<Label*>(stmt.get());
                symbolTable.addLabel(label->name, currentAddress);
                std::cout << "Added label " << label->name << " at address " << currentAddress << std::endl;
                break;
            }
            case StatementType::DIRECTIVE: {
                auto directive = static_cast<Directive*>(stmt.get());
                if(directive->name == ".define") {
                    int value = std::stoi(directive->value, nullptr, 0);
                    symbolTable.addDefine(directive->label, value);
                    std::cout << "Added define " << directive->label << " = " << value << std::endl;
                } else if(directive->name == ".word") {
                    currentAddress++;
                    std::cout << "Word directive, incrementing address to " << currentAddress << std::endl;
                }
                break;
            }
            case StatementType::INSTRUCTION:
                currentAddress++;
                std::cout << "Instruction, incrementing address to " << currentAddress << std::endl;
                break;
        }
    }

    std::cout << "\nSecond Pass:\n";
    currentAddress = 0;
    for(const auto& stmt : ast) {
        switch(stmt->type) {
            case StatementType::INSTRUCTION: {
                auto instr = static_cast<Instruction*>(stmt.get());
                std::cout << "Processing instruction: " << instr->opcode 
                        << " " << instr->operand1 
                        << " " << instr->operand2 << std::endl;

                std::string op2 = instr->operand2;
                bool isPseudo = false;
                int resolvedValue = 0;

                    if (instr->opcode == "mv" && (op2.size() > 0 && op2[0] == '=')) {
                        std::string labelName = op2.substr(1);
                    
                    if (symbolTable.hasLabel(labelName)) {
                        resolvedValue = symbolTable.getLabelAddress(labelName);
                        isPseudo = true;
                    }
                    else if (symbolTable.hasDefine(labelName)) {
                        resolvedValue = symbolTable.getDefineValue(labelName);
                        isPseudo = true;
                    }
                    else {
                        throw std::runtime_error("Undefined label or define: " + labelName);
                    }

                    uint16_t address = static_cast<uint16_t>(resolvedValue);
                    uint8_t highByte = (address >> 8) & 0xFF;
                    uint8_t lowByte = address & 0xFF;

                    auto encodedHigh = encoder.encode("mvt", instr->operand1, 
                                                    std::to_string(highByte), 
                                                    currentAddress, 0);
                    auto encodedLow = encoder.encode("add", instr->operand1, 
                                                std::to_string(lowByte), 
                                                currentAddress + 1, 0);

                    machineCode.insert(machineCode.end(), encodedHigh.begin(), encodedHigh.end());
                    machineCode.insert(machineCode.end(), encodedLow.begin(), encodedLow.end());
                    currentAddress += 2;
                    continue;
                }
                else if (symbolTable.hasLabel(op2)) {
                    resolvedValue = symbolTable.getLabelAddress(op2);
                    op2 = std::to_string(resolvedValue);
                }
                else if (symbolTable.hasDefine(op2)) {
                    resolvedValue = symbolTable.getDefineValue(op2);
                    op2 = std::to_string(resolvedValue);
                }

                auto encoded = encoder.encode(instr->opcode, instr->operand1, op2, currentAddress, 0);
                
                std::cout << "Encoded instruction to: ";
                for (auto code : encoded) {
                    std::cout << "0x" << std::hex << code << " ";
                }
                std::cout << std::endl;

                machineCode.insert(machineCode.end(), encoded.begin(), encoded.end());
                currentAddress += encoded.size();
                break;
            }
            case StatementType::DIRECTIVE:
            case StatementType::LABEL:
                break;
        }
    }

    std::cout << "\nFinal machine code:\n";
    for(size_t i = 0; i < machineCode.size(); i++) {
        std::cout << std::hex << i << ": 0x" << std::setw(4) 
                  << std::setfill('0') << machineCode[i] << '\n';
    }

    return 0;
}