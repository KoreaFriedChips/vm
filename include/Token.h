#ifndef TOKEN_H
#define TOKEN_H

#include "TokenType.h"
#include <string>

class Token {
public:
    TokenType type;
    std::string text;
    int position; // Starting position in the line

    Token(TokenType type, const std::string& text, int position)
        : type(type), text(text), position(position) {}
};

#endif 
