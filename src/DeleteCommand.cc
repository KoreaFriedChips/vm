#include "DeleteCommand.h"
#include "ncurses.h"
#include <iostream>

DeleteCommand::DeleteCommand(Editor& editor, bool type, char command, int numOfTimes)
    : editor(editor), type(type), command(command), numOfTimes(numOfTimes) {
    cursorYBefore = editor.getCursorY();
    cursorXBefore = editor.getCursorX();
}

void DeleteCommand::execute() {
    for (int cnt = 0; cnt < numOfTimes; cnt++) {
        deletedChar = editor.getCharacterAtCursor();
        editor.deleteCharacter(type);
    }
    if (command == 's') {
        editor.switchMode(Mode::Insert);
    }
    if (!editor.fileHistory.empty() && editor.textBuffer != editor.fileHistory.top().first) {
        editor.fileHistory.push({editor.textBuffer, {cursorYBefore, cursorXBefore}});
    }
}

void DeleteCommand::undo() {
    editor.setCursorPosition(cursorYBefore, cursorXBefore);
    editor.insertCharacter(deletedChar[0]);
}

std::string DeleteCommand::getType() const {
    return "DeleteCommand";
}
