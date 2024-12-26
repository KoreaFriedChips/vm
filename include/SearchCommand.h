#ifndef SEARCHCOMMAND_H
#define SEARCHCOMMAND_H

#include "Editor.h"
#include <string>

class SearchCommand {
public:
    SearchCommand(Editor& editor, int numOfTimes);
    ~SearchCommand();

    void execute(const std::string& word, char direction);

private:
    Editor& editor;
    int numOfTimes;

    std::string captureSearchInput(char direction);
    bool performSearch(const std::string& query, char direction, int& foundY, int& foundX);
    void highlightPattern(int y, int x, const std::string& query, char direction);
};

#endif 
