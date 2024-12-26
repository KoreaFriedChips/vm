#ifndef MOTIONCOMMAND_H
#define MOTIONCOMMAND_H

#include "ICommand.h"
#include "Editor.h"
#include <string>

class MotionCommand : public ICommand {
public:
    MotionCommand(Editor& editor, char type, int numOfTimes);
    ~MotionCommand() = default;

    void execute() override;
    void undo() override;
    std::string getType() const override;

private:
    Editor& editor;
    char type; // the actual command
    int numOfTimes;

    void w();
    void dollarSign();
};

#endif 
