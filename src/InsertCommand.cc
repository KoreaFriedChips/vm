#include "InsertCommand.h"

InsertCommand::InsertCommand(Editor& editor, char character)
    : editor(editor), characterInserted(character) {
    cursorYBefore = editor.getCursorY();
    cursorXBefore = editor.getCursorX();
}

void InsertCommand::execute() {
    editor.insertCharacter(characterInserted);
}

void InsertCommand::undo() {
    editor.setCursorPosition(cursorYBefore, cursorXBefore);
    editor.deleteCharacter(true);
}

std::string InsertCommand::getType() const {
    return "InsertCommand";
}
