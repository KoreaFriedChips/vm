#include "Lexer.h"
#include <cctype>
#include <algorithm>
#include <unordered_map>

Lexer::Lexer() : inMultiLineComment(false) {
    std::vector<std::string> keywordList = {
        "alignas", "alignof", "and", "and_eq", "asm", "atomic_cancel",
        "atomic_commit", "atomic_noexcept", "auto", "bitand", "bitor",
        "bool", "break", "case", "catch", "char", "char8_t", "char16_t",
        "char32_t", "class", "compl", "concept", "const", "consteval",
        "constexpr", "constinit", "const_cast", "continue", "co_await",
        "co_return", "co_yield", "decltype", "default", "delete", "do",
        "double", "dynamic_cast", "else", "enum", "explicit", "export",
        "extern", "false", "float", "for", "friend", "goto", "if",
        "inline", "int", "long", "mutable", "namespace", "new", "noexcept",
        "not", "not_eq", "nullptr", "operator", "or", "or_eq", "private",
        "protected", "public", "reflexpr", "register", "reinterpret_cast",
        "requires", "return", "short", "signed", "sizeof", "static",
        "static_assert", "static_cast", "struct", "switch", "synchronized",
        "template", "this", "thread_local", "throw", "true", "try",
        "typedef", "typeid", "typename", "union", "unsigned", "using",
        "virtual", "void", "volatile", "wchar_t", "while", "xor",
        "xor_eq"
    };
    keywords.insert(keywordList.begin(), keywordList.end());
}

bool Lexer::isKeyword(const std::string& word) {
    return keywords.find(word) != keywords.end();
}

bool Lexer::isIdentifierChar(char c) {
    return std::isalnum(c) || c == '_';
}

bool Lexer::isOperatorChar(char c) {
    std::string operators = "+-*/%=<>!&|^~?:.";
    return operators.find(c) != std::string::npos;
}

bool Lexer::isPunctuationChar(char c) {
    std::string punctuation = "();{}[],";
    return punctuation.find(c) != std::string::npos;
}

std::vector<Token> Lexer::tokenize(const std::string& line) {
    std::vector<Token> tokens;
    int i = 0;
    int length = line.length();

    while (i < length) {
        if (inMultiLineComment) {
            size_t endComment = line.find("*/", i);
            if (endComment != std::string::npos) {
                std::string comment = line.substr(i, endComment + 2 - i);
                tokens.emplace_back(TokenType::Comment, comment, i);
                i = endComment + 2;
                inMultiLineComment = false;
            }
            else {
                std::string comment = line.substr(i);
                tokens.emplace_back(TokenType::Comment, comment, i);
                break; 
            }
        }
        else if (line.compare(i, 2, "//") == 0) {
            std::string comment = line.substr(i);
            tokens.emplace_back(TokenType::Comment, comment, i);
            break; 
        }
        else if (line.compare(i, 2, "/*") == 0) {
            size_t endComment = line.find("*/", i + 2);
            if (endComment != std::string::npos) {
                std::string comment = line.substr(i, endComment + 2 - i);
                tokens.emplace_back(TokenType::Comment, comment, i);
                i = endComment + 2;
            }
            else {
                std::string comment = line.substr(i);
                tokens.emplace_back(TokenType::Comment, comment, i);
                inMultiLineComment = true;
                break; 
            }
        }
        else if (line[i] == '"' || line[i] == '\'') {
            char quote = line[i];
            int start = i;
            i++; 
            bool escape = false;
            while (i < length) {
                if (line[i] == '\\' && !escape) {
                    escape = true;
                }
                else if (line[i] == quote && !escape) {
                    i++; 
                    break;
                }
                else {
                    escape = false;
                }
                i++;
            }
            std::string literal = line.substr(start, i - start);
            TokenType type = (quote == '"') ? TokenType::StringLiteral : TokenType::StringLiteral; // Treat char literals as string literals for highlighting
            tokens.emplace_back(type, literal, start);
        }
        else if (line[i] == '#') {
            int start = i;
            while (i < length && line[i] != '\n') {
                i++;
            }
            std::string directive = line.substr(start, i - start);
            tokens.emplace_back(TokenType::PreprocessorDirective, directive, start);
        }
        else if (std::isdigit(line[i])) {
            int start = i;
            while (i < length && (std::isdigit(line[i]) || line[i] == '.' || line[i] == 'x' || line[i] == 'X')) {
                i++;
            }
            std::string number = line.substr(start, i - start);
            tokens.emplace_back(TokenType::NumericLiteral, number, start);
        }
        else if (isOperatorChar(line[i])) {
            int start = i;
            if (i + 1 < length) {
                std::string op = line.substr(i, 2);
                static std::unordered_set<std::string> multiCharOps = {
                    "==", "!=", "<=", ">=", "++", "--", "&&", "||", "+=", "-=", "*=", "/=", "%=", "&=", "|=", "^=", "->", "::"
                };
                if (multiCharOps.find(op) != multiCharOps.end()) {
                    tokens.emplace_back(TokenType::Operator, op, start);
                    i += 2;
                    continue;
                }
            }
            tokens.emplace_back(TokenType::Operator, std::string(1, line[i]), i);
            i++;
        }
        else if (isPunctuationChar(line[i])) {
            std::string punct(1, line[i]);

            static std::unordered_map<char, char> matchingDelimiters = {
                {'}', '{'},
                {']', '['},
                {')', '('}
            };

            if (line[i] == '{' || line[i] == '[' || line[i] == '(') {
                delimiterStack.push(line[i]);
                tokens.emplace_back(TokenType::Punctuation, punct, i);
            }
            else if (line[i] == '}' || line[i] == ']' || line[i] == ')') {
                if (!delimiterStack.empty() && delimiterStack.top() == matchingDelimiters[line[i]]) {
                    delimiterStack.pop();
                    tokens.emplace_back(TokenType::Punctuation, punct, i);
                }
                else {
                    tokens.emplace_back(TokenType::MismatchedBrace, punct, i); 
                }
            }
            else {
                tokens.emplace_back(TokenType::Punctuation, punct, i);
            }
            i++;
        }
        else if (std::isalpha(line[i]) || line[i] == '_') {
            int start = i;
            while (i < length && isIdentifierChar(line[i])) {
                i++;
            }
            std::string word = line.substr(start, i - start);
            bool isKw = isKeyword(word);

            if (isKw) {
                if (word == "typename" || word == "concept") {
                    if (!delimiterStack.empty() && delimiterStack.top() == '<') {
                        tokens.emplace_back(TokenType::Keyword, word, start);
                    }
                    else {
                        tokens.emplace_back(TokenType::Identifier, word, start);
                    }
                }
                else {
                    tokens.emplace_back(TokenType::Keyword, word, start);
                }
            }
            else {
                tokens.emplace_back(TokenType::Identifier, word, start);
            }
        }
        else {
            tokens.emplace_back(TokenType::PlainText, std::string(1, line[i]), i);
            i++;
        }
    }
    return tokens;
}
