#include "Parser.h"

std::unique_ptr<Statement> Parser::parseStatement() {
    Token current = peek();

    if (current.type == TokenType::LABEL) {
        return parseLabel();
    }

    if (current.type == TokenType::DIRECTIVE) {
        return parseDirective();
    }

    if (current.type == TokenType::INSTRUCTION) {
        return parseInstruction();
    }

    if (current.type == TokenType::COMMENT) {
        advance();
        return nullptr;
    }

    throw std::runtime_error("Unexpected token at line " + 
                           std::to_string(current.line) + 
                           ", column " + std::to_string(current.column));
}

std::unique_ptr<Instruction> Parser::parseInstruction() {
    Token instr = advance();
    std::string opcode = instr.value;
    std::string operand1, operand2;
    bool hasComma = false;
    bool isLabelImmediate = false;

    if (opcode[0] == 'b') {
        if (!check(TokenType::LABEL_REF)) {
            throw std::runtime_error("Expected label after branch instruction at line " + 
                                   std::to_string(instr.line));
        }
        operand1 = advance().value;
        return std::make_unique<Instruction>(opcode, operand1, "", false, false,
                                           instr.line, instr.column);
    }

    if (opcode == "push" || opcode == "pop") {
        if (!check(TokenType::REGISTER)) {
            throw std::runtime_error("Expected register after " + opcode + 
                                   " at line " + std::to_string(instr.line));
        }
        operand1 = advance().value;
        return std::make_unique<Instruction>(opcode, operand1, "", false, false,
                                           instr.line, instr.column);
    }

    if (!check(TokenType::REGISTER)) {
        throw std::runtime_error("Expected register at line " + 
                               std::to_string(peek().line));
    }
    operand1 = advance().value;

    if (!match(TokenType::COMMA)) {
        throw std::runtime_error("Expected comma after " + operand1 + 
                               " at line " + std::to_string(peek().line));
    }
    hasComma = true;

    if (opcode == "ld" || opcode == "st") {
        if (!match(TokenType::BRACKET_OPEN)) {
            throw std::runtime_error("Expected '[' after comma at line " + 
                                   std::to_string(peek().line));
        }
        if (!check(TokenType::REGISTER)) {
            throw std::runtime_error("Expected register inside brackets at line " + 
                                   std::to_string(peek().line));
        }
        operand2 = advance().value;
        if (!match(TokenType::BRACKET_CLOSE)) {
            throw std::runtime_error("Expected ']' after register at line " + 
                                   std::to_string(peek().line));
        }
    }
    else if (opcode == "lsl" || opcode == "lsr" || opcode == "asr" || opcode == "ror") {
        if (check(TokenType::REGISTER) || check(TokenType::NUMBER)) {
            operand2 = advance().value;
        } else {
            throw std::runtime_error("Expected register or immediate at line " + 
                                   std::to_string(peek().line));
        }
    }
    else if (opcode == "mv") {
        if (check(TokenType::LABEL_IMMEDIATE)) {
            Token labelImm = advance();
            operand2 = labelImm.value;
            isLabelImmediate = true;
        }
        else if (check(TokenType::REGISTER)) {
            operand2 = advance().value;
        } else if (check(TokenType::NUMBER) || check(TokenType::LABEL_REF)) {
            operand2 = advance().value;
        } 
        
        else {
            throw std::runtime_error("Expected register, numeric immediate, or label immediate at line " + 
                                   std::to_string(peek().line));
        }
    }
    else {
        if (check(TokenType::REGISTER)) {
            operand2 = advance().value;
        } else if(check(TokenType::NUMBER) || check(TokenType::LABEL_REF)) {
            operand2 = advance().value;
        } else {
            throw std::runtime_error("Expected register or numeric immediate at line " + 
                                   std::to_string(peek().line));
        }
    }

    return std::make_unique<Instruction>(opcode, operand1, operand2, hasComma, isLabelImmediate,
                                       instr.line, instr.column);
}

std::unique_ptr<Directive> Parser::parseDirective() {
    Token dir = advance();
    std::string name = dir.value;
    std::string label, value;

    if (name == ".define") {
        if (!check(TokenType::LABEL_REF)) {
            throw std::runtime_error("Expected label after .define at line " + 
                                   std::to_string(dir.line));
        }
        label = advance().value;

        if (!check(TokenType::NUMBER)) {
            throw std::runtime_error("Expected number after .define " + label + 
                                   " at line " + std::to_string(dir.line));
        }
        value = advance().value;
    }
    else if (name == ".word") {
        if (!check(TokenType::NUMBER)) {
            throw std::runtime_error("Expected number after .word at line " + 
                                   std::to_string(dir.line));
        }
        value = advance().value;
    }

    return std::make_unique<Directive>(name, label, value, dir.line, dir.column);
}

std::unique_ptr<Label> Parser::parseLabel() {
    Token label = advance();
    return std::make_unique<Label>(label.value, label.line, label.column);
}

std::vector<std::unique_ptr<Statement>> Parser::parse() {
    std::vector<std::unique_ptr<Statement>> statements;
    
    while (!isAtEnd()) {
        try {
            if (peek().type == TokenType::END_OF_FILE) break;
            
            auto stmt = parseStatement();
            if (stmt) {
                statements.push_back(std::move(stmt));
            }
        } catch (const std::exception& e) {
            throw std::runtime_error("Parse error at line " + 
                std::to_string(peek().line) + ": " + e.what());
        }
    }

    return statements;
}

Token Parser::peek() const {
    if (isAtEnd()) return Token(TokenType::END_OF_FILE, "", -1, -1);
    return tokens[current];
}

Token Parser::advance() {
    if (!isAtEnd()) current++;
    return previous();
}

Token Parser::previous() const {
    return tokens[current - 1];
}

bool Parser::isAtEnd() const {
    return current >= tokens.size();
}

bool Parser::match(TokenType type) {
    if (check(type)) {
        advance();
        return true;
    }
    return false;
}

bool Parser::check(TokenType type) const {
    if (isAtEnd()) return false;
    return peek().type == type;
}