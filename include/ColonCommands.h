#ifndef COLONCOMMANDS_H
#define COLONCOMMANDS_H

#include "Editor.h"
#include <string>

class ColonCommands {
public:
    ColonCommands(Editor& editor);
    void execute(const std::string& commandStr);

private:
    Editor& editor;
    bool isNumber(const std::string& commandStr);
    void saveFile(const std::string& filename);
    void quit();
    void quitForced();
    void loadFile(const std::string& filename);
    void readFile(const std::string& filename);
};

#endif
