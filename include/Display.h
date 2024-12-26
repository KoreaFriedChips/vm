#ifndef DISPLAY_H
#define DISPLAY_H

#include <string>
#include <vector>
#include <ncurses.h>
#include "SyntaxHighlighter.h" 

class Display {
public:
    Display();
    ~Display();

    void initialize();
    void renderText(const std::vector<std::string>& textBuffer, int rows);
    void renderCC(const std::vector<std::string>& textBuffer, int rows);

    void clear();
    void refresh();
    WINDOW* getWindow() const;
    void getWindowSize(int& rows, int& cols) const;

    SyntaxHighlighter syntaxHighlighter; 


private:
    WINDOW* window;
};

#endif 
