#ifndef DELETECOMMAND_H
#define DELETECOMMAND_H

#include "ICommand.h"
#include "Editor.h"
#include <string>

class DeleteCommand : public ICommand {
public:
    DeleteCommand(Editor& editor, bool type, char command, int numOfTimes);
    ~DeleteCommand() = default;

    void execute() override;
    void undo() override;
    std::string getType() const override;

private:
    Editor& editor;
    std::string deletedChar;
    int cursorYBefore;
    int cursorXBefore;
    bool type; // true represents backspace, false represents delete
    char command;
    int numOfTimes;
};

#endif
