#ifndef COMMANDPARSER_H
#define COMMANDPARSER_H

#include "ICommand.h"
#include "Editor.h"
#include <memory>

class CommandParser {
public:
    CommandParser(Editor& editor);

    // Parses input character and returns a command
    std::shared_ptr<ICommand> parseInput(int ch);

private:
    Editor& editor;

    // for repeating last search for (f, F)
    int upper; // -1 represents no search yet, 0 represents f, 1 represents F
    char repeatChar; // dummy char, won't run unlesss upper >= 0
    // for repeating last search for (/, ?)
    char dir; // which char
    std::string query; // the query

    void a();
    void A();
    void cc(int numOfTimes);
    void cMotion();
    void deleteRange(int startY, int startX, int endY, int endX);
    void deleteWord();
    void dd();
    void dMotion();
    void findCharOnLine(bool lower, char c, int mult = 0);
    void i();
    void I();
    void J();
    void openLine(bool lower);
    void paste(bool lower);
    void r(char c, int numOfTimes);
    void R();
    void copyCharacters(Editor &editor, char ch, int count);
    void copyWord(int multiplier, int direction);
    void copyLines(Editor &editor, int count);
    void yMotion();
    void ctrlG();
    void percentCommand(int percentage);
    void findMatchingBracket(int startX, int startY, bool dir);

    void colonCommand();
    void callMotionCommand(Editor& editor, char ch, int numOfTimes);
    void callDeleteCommand(Editor& editor, bool type, char ch, int numOfTimes);
    void callSearchCommand(Editor& editor, char direction, int numOfTimes);
    void tempFun();
};

#endif
