#include "ColonCommands.h"
#include <sstream>
#include <fstream>
#include <ncurses.h> 

ColonCommands::ColonCommands(Editor& editor)
    : editor(editor) {
}

void ColonCommands::execute(const std::string& commandStr) {
    size_t start = commandStr.find_first_not_of(" \t");
    size_t end = commandStr.find_last_not_of(" \t");
    std::string trimmedCommand = (start == std::string::npos) ? "" :
                                 commandStr.substr(start, end - start + 1);

    // Handle empty command
    if (trimmedCommand.empty()) {
        editor.setStatusMessage("No command entered.", true);
        return;
    }

    std::istringstream iss(trimmedCommand);
    std::string command;
    iss >> command;

    if (command == "w") {
        std::string filename;
        iss >> filename;
        if (!filename.empty()) {
            saveFile(filename);
        } else {
            if (!editor.getFilename().empty()) {
                saveFile(editor.getFilename());
            } else {
                editor.setStatusMessage("No filename specified. Use :w <filename> to save.", true);
            }
        }
    }
    else if (command == "q") {
        if (editor.getIsModified()) { 
            ungetch('\n');
            editor.setStatusMessage("E37: No write since last change (add ! to override)", true);
            editor.setCursorPosition(editor.cursorY, editor.cursorX);
        } else {
            quit();
        }
    }
    else if (command == "q!") {
        quitForced();
    }
    else if (command == "wq") {
        // Save and quit
        if (!editor.getFilename().empty()) {
            saveFile(editor.getFilename());
            quit();
        } else {
            editor.setStatusMessage("No filename specified. Use :w <filename> to save before quitting.", true);
        }
    }
    else if (command == "0") {
        // Move cursor to the beginning of the file
        editor.moveCursorToStart();
        ungetch('\n');
        editor.setStatusMessage("Moved to the beginning of the file.", false);
    }
    else if (command == "$") {
        // Move cursor to the end of the file
        editor.moveCursorToEnd();
        ungetch('\n');
        editor.setStatusMessage("Moved to the end of the file.", false);
    }
    else if (command == "r") {
        std::string filename;
        iss >> filename;
        if (!filename.empty()) {
            readFile(filename);
        } else {
            editor.setStatusMessage("Usage: :r <filename>", true);
        }
    }
    else if (isNumber(command)) {
        int lineNumber = 0;
        for (int i = 0; i < (int)commandStr.size(); i++) {
            lineNumber = lineNumber * 10 + (commandStr[i] - '0');
        }
        lineNumber = std::min((int)editor.textBuffer.size(), lineNumber);
        editor.setCursorPosition(lineNumber - 1, 0);
        ungetch('\n');
    }
    else {
        editor.setStatusMessage("Unknown command: " + trimmedCommand, true);
    }
}

bool ColonCommands::isNumber(const std::string& commandStr) {
    for (int i = 0; i < (int)commandStr.size(); i++) {
        if (commandStr[i] < '0' || commandStr[i] > '9') {
            return false;
        }
    }
    return true;
}

void ColonCommands::saveFile(const std::string& filename) {
    if (editor.saveFile(filename)) {
        editor.setStatusMessage("File saved: " + filename, false); 
    } else {
        editor.setStatusMessage("Error saving file: " + filename, true); 
    }
}

void ColonCommands::quit() {
    editor.setIsRunning(false);
}

void ColonCommands::quitForced() {
    editor.setIsRunning(false);
}

void ColonCommands::loadFile(const std::string& filename) {
    if (editor.loadFile(filename)) {
        editor.setStatusMessage("File loaded: " + filename, false); 
    } else {
        editor.setStatusMessage("Error loading file: " + filename, true);
    }
}

void ColonCommands::readFile(const std::string& filename) {
    std::ifstream infile(filename);
    if (!infile.is_open()) {
        editor.setStatusMessage("Error: Cannot open file '" + filename + "'.", true);
        return;
    }

    std::vector<std::string> fileLines;
    std::string line;
    while (std::getline(infile, line)) {
        fileLines.push_back(line);
    }
    infile.close();

    if (fileLines.empty()) {
        editor.setStatusMessage("File '" + filename + "' is empty.", true);
        return;
    }

    editor.insertLinesBelowCursor(fileLines);
    editor.setStatusMessage("Inserted content from '" + filename + "'.", false);
}
