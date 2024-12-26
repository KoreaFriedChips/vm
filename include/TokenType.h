#ifndef TOKENTYPE_H
#define TOKENTYPE_H

enum class TokenType {
    Keyword,
    NumericLiteral,
    StringLiteral,
    Identifier,
    Comment,
    PreprocessorDirective,
    Operator,
    Punctuation,
    MismatchedBrace,
    MismatchedBracket,
    MismatchedParenthesis,
    PlainText
};

#endif
