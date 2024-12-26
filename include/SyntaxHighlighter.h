#ifndef SYNTAX_HIGHLIGHTER_H
#define SYNTAX_HIGHLIGHTER_H

#include "Lexer.h"
#include "Token.h"
#include <vector>
#include <string>
#include <utility>

class SyntaxHighlighter {
public:
    SyntaxHighlighter();
    ~SyntaxHighlighter();

    std::vector<std::pair<TokenType, std::string>> highlight(const std::string& line);

private:
    Lexer lexer;
};

#endif
