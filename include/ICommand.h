#ifndef ICOMMAND_H
#define ICOMMAND_H

#include <string>

class ICommand {
public:
    virtual ~ICommand() = default;
    virtual void execute() = 0;
    virtual void undo() = 0;
    virtual std::string getType() const = 0;
};

#endif 
