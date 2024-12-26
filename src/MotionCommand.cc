#include "MotionCommand.h"
#include "Editor.h"

MotionCommand::MotionCommand(Editor& editor, char type, int numOfTimes)
    : editor(editor), type(type), numOfTimes(numOfTimes) {
}

void MotionCommand::execute() {
    for (int cnt = 0; cnt < numOfTimes; cnt++) {
        if (type == 'b') {
            editor.moveCursorBackWord();
        } else if (type == 'w') {
            w();
        } else if (type == '$') {
            if (cnt >= 1) {
                editor.moveCursor(1, 0);
            }
            dollarSign();
        }
        else if (type == ('b' & 0x1F)) {
            editor.moveCursorBackwardFrame();
        }
        else if (type == ('d' & 0x1F)) {
            editor.moveCursorDownFrame();
        }
        else if (type == ('u' & 0x1F)) {
            editor.moveCursorUpFrame();
        }
        else if (type == ('f' & 0x1F)) {
            editor.moveCursorForwardFrame();
        }
        else if (type == 'h') {
            editor.moveCursor(0, -1);
        }
        else if (type == 'j') {
            editor.moveCursor(1, 0);
        }
        else if (type == 'k') {
            editor.moveCursor(-1, 0);
        }
        else if (type == 'l') {
            editor.moveCursor(0, 1);
        }
    }
}

void MotionCommand::undo() {
    return;
}

std::string MotionCommand::getType() const {
    return "MotionCommand";
}

// may also have to change it so that w moves to the start of 
// ';', ':', '/', '\', 
void MotionCommand::w() {
    if (editor.textBuffer.empty()) {
        editor.setStatusMessage("Buffer is empty.", true);
        beep();
        return;
    }

    int maxY = editor.textBuffer.size();
    bool flag = false;
    bool newLine = false;
    while (editor.cursorY < maxY && !flag) {
        std::string& currentLine = editor.textBuffer[editor.cursorY];
        int lineLength = currentLine.length();

        int pos = editor.cursorX;

        while (pos < lineLength && !std::isspace(currentLine[pos]) && !newLine) {
            pos++;
        }

        while (pos < lineLength && std::isspace(currentLine[pos])) {
            pos++;
        }

        if (pos < lineLength) {
            editor.setCursorPosition(editor.cursorY, pos);
            editor.setStatusMessage("Moved to the start of the next word.", false);
            flag = true;
        }
        else {
            editor.cursorY++;
            editor.cursorX = 0;
            newLine = true;
        }
    }
    if (!flag) {
        editor.cursorY--;
        editor.cursorX = editor.preferredCursorX;
        beep();
    }
}

void MotionCommand::dollarSign() {
    const std::string& currentLine = editor.textBuffer[editor.cursorY];
    editor.setCursorPosition(editor.cursorY, currentLine.size() - 1);
}