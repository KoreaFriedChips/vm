#include "SyntaxHighlighter.h"

SyntaxHighlighter::SyntaxHighlighter() {}

SyntaxHighlighter::~SyntaxHighlighter() {}

std::vector<std::pair<TokenType, std::string>> SyntaxHighlighter::highlight(const std::string& line) {
    std::vector<std::pair<TokenType, std::string>> highlightedTokens;
    std::vector<Token> tokens = lexer.tokenize(line);

    for (const auto& token : tokens) {
        highlightedTokens.emplace_back(std::make_pair(token.type, token.text));
    }

    return highlightedTokens;
}
