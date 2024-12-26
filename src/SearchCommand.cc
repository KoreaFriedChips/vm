#include "SearchCommand.h"
#include <ncurses.h>
#include <algorithm>
#include <cctype>
#include <sstream>
#include <fstream>
#include <ncurses.h>

SearchCommand::SearchCommand(Editor& editor, int numOfTimes)
    : editor(editor), numOfTimes(numOfTimes) {}

SearchCommand::~SearchCommand() {}

void SearchCommand::execute(const std::string& wordStr, char direction) {

    std::istringstream iss(wordStr);
    std::string word;
    iss >> word;
    int foundY = -1, foundX = -1;
    for (int i = 0; i < numOfTimes; i++) {
        if (performSearch(word, direction, foundY, foundX)) {
            editor.setCursorPosition(foundY, foundX);
            ungetch('\n');
        } else {
            beep();
            editor.setStatusMessage("E486: Pattern not found: " + word, true);
            break;
        }
    }
}

bool SearchCommand::performSearch(const std::string& query, char direction, int& foundY, int& foundX) {
    if (query.empty())
        return false;

    if (direction == '/') {
        // Forward search
        for (int y = editor.cursorY; y < static_cast<int>(editor.textBuffer.size()); ++y) {
            int startX = (y == editor.cursorY) ? editor.cursorX + 1 : 0;
            size_t pos = editor.textBuffer[y].find(query, startX);
            if (pos != std::string::npos) {
                foundY = y;
                foundX = static_cast<int>(pos);
                editor.setStatusMessage("Found next: " + query, true);
                return true;
            }
        }
        for (int y = 0; y < editor.cursorY; ++y) {
            int startX = (y == editor.cursorY) ? editor.cursorX + 1 : 0;
            size_t pos = editor.textBuffer[y].find(query, startX);
            if (pos != std::string::npos) {
                foundY = y;
                foundX = static_cast<int>(pos);
                editor.setStatusMessage("search hit BOTTOM, continuing at TOP", true);
                return true;
            }
        }
    }
    else if (direction == '?') {
        // Backward search
        for (int y = editor.cursorY; y >= 0; --y) {
            int endX = (y == editor.cursorY) ? editor.cursorX - 1 : editor.textBuffer[y].size() - 1;
            if (endX < 0)
                continue;
            size_t pos = editor.textBuffer[y].rfind(query, endX);
            if (pos != std::string::npos) {
                foundY = y;
                foundX = static_cast<int>(pos);
                editor.setStatusMessage("Found previous: " + query, true);
                return true;
            }
        }
        for (int y = static_cast<int>(editor.textBuffer.size()); y > editor.cursorY; --y) {
            int endX = (y == editor.cursorY) ? editor.cursorX - 1 : editor.textBuffer[y].size() - 1;
            if (endX < 0)
                continue;
            size_t pos = editor.textBuffer[y].rfind(query, endX);
            if (pos != std::string::npos) {
                foundY = y;
                foundX = static_cast<int>(pos);
                editor.setStatusMessage("search hit TOP, continuing at BOTTOM", true);
                return true;
            }
        }
    }

    return false; 
}

