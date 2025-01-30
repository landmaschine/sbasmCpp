#include "Parser.h"

Token Parser::peek() const {
    if(isAtEnd()) return Token(TokenType::END_OF_FILE, "", -1, -1);
    return tokens[current];
}

Token Parser::advance() {
    if(!isAtEnd()) current++;
    return previous();
}

bool Parser::isAtEnd() const {
    return current >= tokens.size() || tokens[current].type == TokenType::END_OF_FILE;
}

Token Parser::previous() const {
    return tokens[current - 1];
}

bool Parser::match(TokenType type) {
    if(check(type)) {
        advance();
        return true;
    }
    return false;
}

bool Parser::check(TokenType type) const {
    if(isAtEnd()) return false;
    return peek().type == type;
}

std::unique_ptr<Statement> Parser::parseStatement() {
    Token current = peek();

    if(current.type == TokenType::LABEL) {
        return parseLabel();
    }

    if(current.type == TokenType::DIRECTIVE) {
        return parseDirective();
    }

    if(current.type == TokenType::INSTRUCTION) {
        return parseInstruction();
    }

    if(current.type == TokenType::COMMENT || current.type == TokenType::INVALID) {
        advance();
        return nullptr;
    }

    throw std::runtime_error("Unexpected token at line " + std::to_string(current.line) + ", column " + std::to_string(current.column));
}

std::unique_ptr<Instruction> Parser::parseInstruction() {
    Token instr = advance();
    std::string opcode = instr.value;
    std::string operand1, operand2;
    bool hasComma = false;

    if (check(TokenType::REGISTER) || check(TokenType::LABEL_REF)) {
        operand1 = advance().value;
    } else {
        throw std::runtime_error("Missing first operand for " + opcode + " at line " + std::to_string(instr.line));
    }

    if (check(TokenType::COMMA)) {
        advance();
        hasComma = true;
    } else {
        throw std::runtime_error("Expected comma after first operand for " + opcode + " at line " + std::to_string(instr.line));
    }

    if (opcode == "ld" || opcode == "st") {
        if (!match(TokenType::BRACKET_OPEN)) {
            throw std::runtime_error("Expected '[' after comma for " + opcode + " at line " + std::to_string(peek().line));
        }

        if (check(TokenType::REGISTER)) {
            operand2 = advance().value;
        } else {
            throw std::runtime_error("Expected register inside brackets for " + opcode + " at line " + std::to_string(peek().line));
        }

        if (!match(TokenType::BRACKET_CLOSE)) {
            throw std::runtime_error("Expected ']' after register for " + opcode + " at line " + std::to_string(peek().line));
        }
    } else {
        if (check(TokenType::REGISTER) || check(TokenType::NUMBER) || check(TokenType::LABEL_REF)) {
            operand2 = advance().value;
        } else {
            throw std::runtime_error("Unexpected token in second operand for " + opcode + " at line " + std::to_string(peek().line));
        }
    }

    return std::make_unique<Instruction>(opcode, operand1, operand2, hasComma, instr.line, instr.column);
}

std::unique_ptr<Directive> Parser::parseDirective() {
    Token dir = advance();
    std::string name = dir.value;
    std::string label, value;

    if(name == ".define" && check(TokenType::LABEL_REF)) {
        label = advance().value;
        if(check(TokenType::NUMBER)) {
            value = advance().value;
        }
    }

    else if(name == ".word" && check(TokenType::NUMBER)) {
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

    while(!isAtEnd()) {
        try {
            auto stmt = parseStatement();
            if(stmt) {
                statements.push_back(std::move(stmt));
            }
        } catch(const std::exception& e) {
            std::cerr << "Error: " << e.what() << std::endl;
            std::exit(1);
        }
    }

    return statements;
}
