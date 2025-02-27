// ----------------------------------------------------------------------------
// Author: LeonW
// Date: February 3, 2025
// ----------------------------------------------------------------------------

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
    NUMBER_IMMEDIATE,
    LABEL,           
    LABEL_REF,       
    LABEL_IMMEDIATE, 
    COMMA,           
    BRACKET_OPEN,    
    BRACKET_CLOSE,   
    DIRECTIVE,       
    COMMENT,         
    END_OF_FILE,     
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
        "mvt", "add", "sub", "ld", "pop", "st", "push", "and", "xor",
        "cmp", "lsl", "lsr", "asr", "ror"
    };

    bool isInstruction(const std::string& str) {
        return std::find(instructions.begin(), instructions.end(), str) != instructions.end();
    }

    bool isIdentifierChar(char c) {
        return std::isalnum(c) || c == '_' || c == '$';
    }

    void skipWhitespace();
    Token parseIdentifier();
    int64_t parseNumberValue(const std::string& str);

public:
    Lexer(std::string input) : input(std::move(input)), position(0), line(1), column(1) {}
    
    Token nextToken();
    std::vector<Token> tokenize();
};