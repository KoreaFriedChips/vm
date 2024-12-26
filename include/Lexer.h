#ifndef LEXER_H
#define LEXER_H

#include "Token.h"
#include <vector>
#include <string>
#include <unordered_set>
#include <stack>

class Lexer {
public:
    Lexer();

    std::vector<Token> tokenize(const std::string& line);

private:
    std::unordered_set<std::string> keywords;

    bool isKeyword(const std::string& word);
    bool isIdentifierChar(char c);
    bool isOperatorChar(char c);
    bool isPunctuationChar(char c);

    bool inMultiLineComment;
    std::stack<char> delimiterStack;
};

#endif
