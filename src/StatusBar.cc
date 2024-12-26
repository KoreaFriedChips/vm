#include "StatusBar.h"
#include <string>
#include <ncurses.h>

StatusBar::StatusBar() : leftText(""), rightText(""), statusMessage(""), isErrorMessage(false) {}

StatusBar::~StatusBar() {}

void StatusBar::setLeftText(const std::string& text) {
    leftText = text;
}

void StatusBar::setRightText(const std::string& text) {
    rightText = text;
}

void StatusBar::setStatusMessage(const std::string& message, bool isError) {
    statusMessage = message;
    isErrorMessage = isError;
}

void StatusBar::render(WINDOW* window, int rows, int cols) {
    mvwhline(window, rows - 1, 0, ' ', cols);

    std::string left;
    int leftColorPair = 1; 

    if (!statusMessage.empty()) {
        left = statusMessage;
        if (isErrorMessage) {
            leftColorPair = 3; 
        } else {
            leftColorPair = 4; 
        }
    } else {
        left = leftText;
        leftColorPair = 1; 
    }

    wattron(window, COLOR_PAIR(leftColorPair));
    mvwprintw(window, rows - 1, 0, "%s", left.c_str());
    wattroff(window, COLOR_PAIR(leftColorPair));

    wattron(window, COLOR_PAIR(leftColorPair));
    mvwprintw(window, rows - 1, cols - rightText.length(), "%s", rightText.c_str());
    wattroff(window, COLOR_PAIR(leftColorPair));

    wrefresh(window);
}
