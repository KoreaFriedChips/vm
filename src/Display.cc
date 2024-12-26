#include "Display.h"
#include <ncurses.h>
#include <iostream>

#define COLOR_KEYWORD 16
#define COLOR_NUM_LIT 17
#define COLOR_STR_LIT 18
#define COLOR_ID 19
#define COLOR_COMMENTS 20
#define COLOR_PREP 21


Display::Display() : window(nullptr) {}

Display::~Display() {
    if (window) {
        delwin(window);
    }
    endwin();
}

void Display::initialize() {
    initscr();            
    cbreak();       
    noecho();             
    keypad(stdscr, TRUE); 
    curs_set(1);        

    // Initialize color pairs
    if (has_colors() == FALSE) {
        endwin();
        std::cerr << "Your terminal does not support color\n";
        exit(1);
    }
    start_color();
    init_pair(1, COLOR_WHITE, COLOR_BLUE);    // Status bar text on blue background
    init_pair(2, COLOR_WHITE, COLOR_BLACK);   // Editor content: white text on black background
    init_pair(3, COLOR_WHITE, COLOR_RED);     // Error messages: red text on black background
    init_pair(4, COLOR_GREEN, COLOR_BLACK);   // Success messages: green text on black background
    init_pair(5, COLOR_BLUE, COLOR_BLACK);    // Tilde lines: blue text on black background

    init_color(COLOR_KEYWORD, 238, 395, 523);
    init_pair(6, COLOR_KEYWORD, COLOR_BLACK);      // Keywords
    init_color(COLOR_NUM_LIT, 640, 726, 594);
    init_pair(7, COLOR_NUM_LIT, COLOR_BLACK);   // Numeric literals
    init_color(COLOR_STR_LIT, 777, 551, 453);
    init_pair(8, COLOR_STR_LIT, COLOR_BLACK);    // String literals
    init_color(COLOR_ID, 531, 746, 859);
    init_pair(9, COLOR_ID, COLOR_BLACK);     // Identifiers
    init_color(COLOR_COMMENTS, 387, 559, 313);
    init_pair(10, COLOR_COMMENTS, COLOR_BLACK); // Comments
    init_color(COLOR_PREP, 769, 523, 750);
    init_pair(11, COLOR_PREP, COLOR_BLACK);    // Preprocessor directives
    init_pair(12, COLOR_WHITE, COLOR_BLACK);   // Operators and punctuation
    init_pair(13, COLOR_RED, COLOR_BLACK);     // Mismatched braces, brackets, parentheses

    int rows, cols;
    getmaxyx(stdscr, rows, cols);
    window = newwin(rows, cols, 0, 0);

    scrollok(window, FALSE);

    refresh();
}

void Display::renderText(const std::vector<std::string>& textBuffer, int rows) {
    // Render the text buffer
    for (int i = 0; i < rows - 1; ++i) { // Subtract 1 for the status bar
        if (i < static_cast<int>(textBuffer.size())) {
            // Use color pair 2 for main text
            wattron(window, COLOR_PAIR(2));
            mvwprintw(window, i, 0, "%s", textBuffer[i].c_str());
            wattroff(window, COLOR_PAIR(2));
        } else {
            // Use color pair 5 for tilde lines
            wattron(window, COLOR_PAIR(5));
            mvwprintw(window, i, 0, "~");
            wattroff(window, COLOR_PAIR(5));
        }
    }
}

void Display::renderCC(const std::vector<std::string>& textBuffer, int rows) {
    werase(window);

    // Render the text buffer with syntax highlighting
    for (int i = 0; i < rows - 1; ++i) { 
        if (i < static_cast<int>(textBuffer.size())) {
            std::string line = textBuffer[i];
            std::vector<std::pair<TokenType, std::string>> highlightedTokens = syntaxHighlighter.highlight(line);

            int x = 0;
            for (const auto& tokenPair : highlightedTokens) {
                TokenType type = tokenPair.first;
                std::string text = tokenPair.second;

                switch (type) {
                    case TokenType::Keyword:
                        wattron(window, COLOR_PAIR(6));
                        break;
                    case TokenType::NumericLiteral:
                        wattron(window, COLOR_PAIR(7));
                        break;
                    case TokenType::StringLiteral:
                        wattron(window, COLOR_PAIR(8));
                        break;
                    case TokenType::Identifier:
                        wattron(window, COLOR_PAIR(9));
                        break;
                    case TokenType::Comment:
                        wattron(window, COLOR_PAIR(10));
                        break;
                    case TokenType::PreprocessorDirective:
                        wattron(window, COLOR_PAIR(11));
                        break;
                    case TokenType::Operator:
                    case TokenType::Punctuation:
                        wattron(window, COLOR_PAIR(12));
                        break;
                    case TokenType::MismatchedBrace:
                    case TokenType::MismatchedBracket:
                    case TokenType::MismatchedParenthesis:
                        wattron(window, COLOR_PAIR(13) | A_BOLD);
                        break;
                    case TokenType::PlainText:
                    default:
                        wattron(window, COLOR_PAIR(2));
                        break;
                }

                mvwprintw(window, i, x, "%s", text.c_str());

                switch (type) {
                    case TokenType::Keyword:
                    case TokenType::NumericLiteral:
                    case TokenType::StringLiteral:
                    case TokenType::Identifier:
                    case TokenType::Comment:
                    case TokenType::PreprocessorDirective:
                    case TokenType::Operator:
                    case TokenType::Punctuation:
                    case TokenType::MismatchedBrace:
                    case TokenType::MismatchedBracket:
                    case TokenType::MismatchedParenthesis:
                    case TokenType::PlainText:
                    default:
                        wattroff(window, COLOR_PAIR(6) | COLOR_PAIR(7) | COLOR_PAIR(8) |
                                 COLOR_PAIR(9) | COLOR_PAIR(10) | COLOR_PAIR(11) |
                                 COLOR_PAIR(12) | COLOR_PAIR(13) | A_BOLD);
                        break;
                }

                x += text.length();
            }
        } else {
            wattron(window, COLOR_PAIR(5));
            mvwprintw(window, i, 0, "~");
            wattroff(window, COLOR_PAIR(5));
        }
    }

    wrefresh(window);
}

void Display::clear() {
    werase(window);
}

void Display::refresh() {
    wrefresh(window);
}

WINDOW* Display::getWindow() const {
    return window;
}

void Display::getWindowSize(int& rows, int& cols) const {
    getmaxyx(stdscr, rows, cols);
}
