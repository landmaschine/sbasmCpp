#pragma once
#include "common.h"

#include <string>
#include <vector>
#include <regex>
#include <cctype>
#include <stdexcept>

enum class TokenType {
    INSTRUCTION,
    REGISTER,
    NUMBER,
    LABEL,
    LABEL_REF,
    COMMA,
    BRACKET_OPEN,
    BRACKET_CLOSE,
    DIRECTIVE,
    COMMENT,
    EOL,
    END_OF_FILE,    // not EOF because EOF is a macro in stdio.h
    INVALID
};

struct Token {
    TokenType type;
    std::string value;
    int line;
    int column;

    Token(TokenType t, std::string v, int l, int c) : type(t), value(std::move(v)), line(l), column(c) {}
};

class Lexer {
private:
    std::string input;
    size_t position;
    int line;
    int column;

    const std::vector<std::string> instructions = {
        "mv", "b", "beq", "bne", "bcc", "bcs", "bpl", "bmi", "bl",
        "mvt", "add", "sub", "ld", "pop", "st", "push", "and",
        "cmp", "lsl", "lsr", "asr", "ror"
    };

    // Helper method
    bool isInstruction(const std::string& str) {
        return std::find(instructions.begin(), instructions.end(), str) != instructions.end();
    }

    //Helper method
    bool isIdentifierChar(char c) {
        return std::isalnum(c) || c == '_' || c == '$';
    }

    void skipWhiteSpace();
    Token parseNumber();
    Token parseIdentifier();

public:
    Lexer(std::string input) : input(std::move(input)), position(0), line(1), column(1) {}
    
    Token nextToken();
    std::vector<Token> tokenize();
};
