#include "common.h"
#include "Lexer/Lexer.h"
#include "Parser/Parser.h"
#include "InstructionEncoder/InstructionEncoder.h"
#include "InstructionEncoder/SymbolTable.h"
#include <fstream>
#include <sstream>
#include <string>
#include <iomanip>

void writeMIF(const std::vector<uint16_t>& machineCode, const std::string& outputFile, int depth = 256);

int main(int argc, const char* argv[]) {
    if(argc < 2) {
        std::cerr << "Usage: " << argv[0] << " <input_file> [output_file]" << std::endl;
        return 1;
    }

    std::string outputFile = (argc >= 3) ? argv[2] : "a.mif";

    std::ifstream file(argv[1]);
    if(!file.is_open()) {
        std::cerr << "Error: Could not open file '" << argv[1] << "'" << std::endl;
        return 1;
    }

    std::stringstream buffer;
    buffer << file.rdbuf();
    std::string input = buffer.str();
    file.close();

    int memoryDepth = 256;
    
    std::istringstream iss(input);
    std::string line;
    while (std::getline(iss, line)) {
        if (line.find("DEPTH") != std::string::npos) {
            size_t pos = line.find('=');
            if (pos != std::string::npos) {
                std::string depthStr = line.substr(pos + 1);
                memoryDepth = std::stoi(depthStr);
            }
        }
    }

    try {
        std::cout << "\n=== Lexical Analysis ===\n";
        Lexer lexer(input);
        std::vector<Token> tokens = lexer.tokenize();

        std::cout << "Tokens:\n";
        for (const auto& token : tokens) {
            std::cout << "Line " << token.line << ", Col " << token.column 
                     << ": Type=" << static_cast<int>(token.type) 
                     << ", Value=\"" << token.value << "\"\n";
        }

        std::cout << "\n=== Parsing ===\n";
        Parser parser(tokens);
        std::vector<std::unique_ptr<Statement>> ast = parser.parse();

        std::cout << "Abstract Syntax Tree:\n";
        for (const auto& stmt : ast) {
            std::cout << "Line " << stmt->line << ", Col " << stmt->column << ": ";
            switch(stmt->type) {
                case StatementType::LABEL: {
                    auto label = static_cast<Label*>(stmt.get());
                    std::cout << "LABEL \"" << label->name << "\"\n";
                    break;
                }
                case StatementType::DIRECTIVE: {
                    auto directive = static_cast<Directive*>(stmt.get());
                    std::cout << "DIRECTIVE " << directive->name;
                    if (!directive->label.empty()) 
                        std::cout << " " << directive->label;
                    if (!directive->value.empty()) 
                        std::cout << " " << directive->value;
                    std::cout << "\n";
                    break;
                }
                case StatementType::INSTRUCTION: {
                    auto instr = static_cast<Instruction*>(stmt.get());
                    std::cout << "INSTRUCTION " << instr->opcode;
                    if (!instr->operand1.empty()) 
                        std::cout << " " << instr->operand1;
                    if (!instr->operand2.empty()) 
                        std::cout << " " << instr->operand2;
                    std::cout << "\n";
                    break;
                }
            }
        }

        SymbolTable symbolTable;
        std::vector<uint16_t> machineCode;
        int currentAddress = 0;
        
        std::cout << "\n=== First Pass: Symbol Collection ===\n";
        for(const auto& stmt : ast) {
            switch(stmt->type) {
                case StatementType::LABEL: {
                    auto label = static_cast<Label*>(stmt.get());
                    std::cout << "Adding label: " << label->name << " at address 0x" 
                             << std::hex << currentAddress << std::dec << "\n";
                    symbolTable.addLabel(label->name, currentAddress);
                    break;
                }
                case StatementType::DIRECTIVE: {
                    auto directive = static_cast<Directive*>(stmt.get());
                    if(directive->name == ".define") {
                        int value = std::stoi(directive->value, nullptr, 0);
                        std::cout << "Adding define: " << directive->label << " = 0x" 
                                 << std::hex << value << std::dec << "\n";
                        symbolTable.addDefine(directive->label, value);
                    } else if(directive->name == ".word") {
                        std::cout << "Word directive at address 0x" 
                                 << std::hex << currentAddress << std::dec << "\n";
                        currentAddress++;
                    }
                    break;
                }
                case StatementType::INSTRUCTION:
                    auto instr = static_cast<Instruction*>(stmt.get());
                    int numWords = 1;

                    if (instr->opcode == "mv" && !instr->operand2.empty() && instr->operand2[0] == '=') {
                        numWords = 2;
                    }

                    std::cout << "Instruction at address 0x" 
                                << std::hex << currentAddress << std::dec 
                                << " (size=" << numWords << ")\n";

                    currentAddress += numWords;
                    break;
            }
        }

        

        std::cout << "\n=== Final Machine Code ===\n";
        for (size_t i = 0; i < machineCode.size(); i++) {
            std::cout << " " << std::hex << std::setw(3) << std::setfill('0') << i 
                     << ":  " << std::setw(4) << std::setfill('0') 
                     << machineCode[i] << std::dec << "\n";
        }

        writeMIF(machineCode, outputFile, memoryDepth);
        std::cout << "\nAssembly completed successfully. Output written to " << outputFile << "\n";

    } catch (const std::exception& e) {
        std::cerr << "\nError: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}

void writeMIF(const std::vector<uint16_t>& machineCode, const std::string& outputFile, int depth) {
    std::ofstream out(outputFile);
    if (!out.is_open()) {
        throw std::runtime_error("Could not open output file: " + outputFile);
    }

    out << "WIDTH = 16;\n";
    out << "DEPTH = " << depth << ";\n";
    out << "ADDRESS_RADIX = HEX;\n";
    out << "DATA_RADIX = HEX;\n\n";
    out << "CONTENT\n";
    out << "BEGIN\n";

    const char* regNames[] = {"r0", "r1", "r2", "r3", "r4", "sp", "lr", "pc"};
    const char* conditions[] = {"b   ", "beq ", "bne ", "bcc ", "bcs ", "bpl ", "bmi ", "bl  "};

    for (size_t i = 0; i < machineCode.size(); i++) {
        // Write address
        out << std::hex << std::setw(1) << i;
        out << std::string(i > 0xf ? 6 : 7, ' ');
        
        // Write instruction word
        out << ": " << std::setw(4) << std::setfill('0') << machineCode[i] << ";"
            << std::string(8, ' ') << "% ";

        uint16_t instr = machineCode[i];
        uint16_t opcode = (instr >> 13) & 0x7;
        uint16_t imm = (instr >> 12) & 0x1;
        uint16_t rX = (instr >> 9) & 0x7;
        uint16_t rY = instr & 0x7;
        uint16_t immediate = instr & 0x1FF;
        
        switch (opcode) {
            case 0: // mv
                if (imm) {
                    out << "mv   " << regNames[rX] << ", #0x" << std::hex << immediate;
                } else {
                    out << "mv   " << regNames[rX] << ", " << regNames[rY];
                }
                break;

            case 1: // branch or mvt
                if (imm) {
                    out << "mvt  " << regNames[rX] << ", #0x" << std::hex << (immediate & 0xFF);
                } else {
                    uint16_t cond = (instr >> 9) & 0x7;
                    int16_t offset = immediate;
                    // Sign extend 9-bit offset
                    if (offset & 0x100) {
                        offset |= 0xFF00;
                    }
                    int target = i + 1 + offset;
                    out << conditions[cond] << "0x" << std::hex << target;
                }
                break;

            case 2: // add
                if (imm) {
                    out << "add  " << regNames[rX] << ", #0x" << std::hex << immediate;
                } else {
                    out << "add  " << regNames[rX] << ", " << regNames[rY];
                }
                break;

            case 3: // sub
                if (imm) {
                    out << "sub  " << regNames[rX] << ", #0x" << std::hex << immediate;
                } else {
                    out << "sub  " << regNames[rX] << ", " << regNames[rY];
                }
                break;

            case 4: // ld/pop
                if (imm) {
                    out << "pop  " << regNames[rX];
                } else {
                    out << "ld   " << regNames[rX] << ", [" << regNames[rY] << "]";
                }
                break;

            case 5: // st/push
                if (imm) {
                    out << "push " << regNames[rX];
                } else {
                    out << "st   " << regNames[rX] << ", [" << regNames[rY] << "]";
                }
                break;

            case 6: // and
                if (imm) {
                    out << "and  " << regNames[rX] << ", #0x" << std::hex << immediate;
                } else {
                    out << "and  " << regNames[rX] << ", " << regNames[rY];
                }
                break;

            case 7: // cmp or shift
                {
                    uint16_t shift_flag = (instr >> 8) & 0x1;
                    uint16_t imm_shift = (instr >> 7) & 0x1;
                    uint16_t shift_type = (instr >> 5) & 0x3;
                    uint16_t shift_amount = instr & 0xF;

                    if (shift_flag) {
                        const char* shift_types[] = {"lsl", "lsr", "asr", "ror"};
                        out << shift_types[shift_type] << "  " << regNames[rX];
                        if (imm_shift) {
                            out << ", #0x" << std::hex << shift_amount;
                        } else {
                            out << ", " << regNames[rY];
                        }
                    } else {
                        if (imm) {
                            // Check if immediate is negative (9-bit signed)
                            if (immediate & 0x100) {
                                immediate |= 0xFF00;
                                out << "cmp  " << regNames[rX] << ", #-0x" << std::hex << (-immediate);
                            } else {
                                out << "cmp  " << regNames[rX] << ", #0x" << std::hex << immediate;
                            }
                        } else {
                            out << "cmp  " << regNames[rX] << ", " << regNames[rY];
                        }
                    }
                }
                break;
        }
        out << " %\n";
    }

    out << "END;\n";
    out.close();
}