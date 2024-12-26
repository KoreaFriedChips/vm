#ifndef INSERTCOMMAND_H
#define INSERTCOMMAND_H

#include "ICommand.h"
#include "Editor.h"
#include <string>

class InsertCommand : public ICommand {
public:
    InsertCommand(Editor& editor, char character);
    ~InsertCommand() = default;

    void execute() override;
    void undo() override;
    std::string getType() const override;

private:
    Editor& editor;
    char characterInserted;
    int cursorYBefore;
    int cursorXBefore;
};

#endif 
