#ifndef STATUSBAR_H
#define STATUSBAR_H

#include <string>
#include <ncurses.h>

class StatusBar {
public:
    StatusBar();
    ~StatusBar();

    void setLeftText(const std::string& text);
    void setRightText(const std::string& text);
    void setStatusMessage(const std::string& message, bool isError = false);

    void render(WINDOW* window, int rows, int cols);

private:
    std::string leftText;
    std::string rightText;
    std::string statusMessage;
    bool isErrorMessage;
};

#endif
