#include "CommandParser.h"
#include "ICommand.h"
#include "InsertCommand.h"
#include "DeleteCommand.h"
#include "ColonCommands.h"
#include "MotionCommand.h"
#include "SearchCommand.h"
#include <ncurses.h>
#include <cctype> 
#include <memory>
#include <iostream>
#include <string>
#include <vector>
#include <stack>
#include <algorithm>
#include <cmath>

CommandParser::CommandParser(Editor& editor) : editor(editor) {
    // for repeating last search for (f, F)
    upper = -1; // -1 represents no search yet, 0 represents f, 1 represents F
    repeatChar = 'x'; // dummy char, won't run unlesss upper >= 0
    editor.commandSeq = "";
}

std::shared_ptr<ICommand> CommandParser::parseInput(int ch) {
    Mode mode = editor.getMode();

    if (mode == Mode::Command) {
        int numOfTimes = -1;
        if (ch > '0' && ch <= '9') {
            numOfTimes = ch - '0';
            ch = getch();
            while (ch >= '0' && ch <= '9') {
                numOfTimes = numOfTimes * 10 + (ch - '0');
                ch = getch();
            }
        }
        if (numOfTimes > 0) {
            editor.lastMult = numOfTimes;
        }        
        else {
            editor.lastMult = 1;
        }
        if (!editor.replaying) {
            editor.commandSeq += std::to_string(editor.lastMult);
            editor.commandSeq += (char)ch;
        }
        
        void (CommandParser::*fun)() = nullptr;

        switch (ch) {
            case 'v': {
                editor.enterVisualModeChar();
                break;
            }
            case 'V': {
                editor.enterVisualModeLine();
                break;
            }
            // Insert Command
            case 'i':
                i();
                break;
            case 'I': {
                I();
                break;
            }
            // Insert Command
            case 'a':
                a();
                break;
            // Insert Command
            case 'A': {
                A();
                break;
            }
            // Insert Command (edit commands should also be at insert, cuz they'll have similar undo)
            case 'J': {
                fun = &CommandParser::J;
                editor.savedCommandSeq = editor.commandSeq;
                editor.commandSeq = "";
                break;
            }
            // Copy Command
            case 'y': {
                yMotion();
                editor.savedCommandSeq = editor.commandSeq;
                editor.commandSeq = "";
                break;
            }
            // Paste Command
            case 'p': {
                for (int cnt = 0; cnt < editor.lastMult; cnt++) paste(true);
                editor.savedCommandSeq = editor.commandSeq;
                editor.commandSeq = "";
                break;
            }
            case 'P': {
                for (int cnt = 0; cnt < editor.lastMult; cnt++) paste(false);
                editor.savedCommandSeq = editor.commandSeq;
                editor.commandSeq = "";
                break;
            }
            // Delete Command
            case 'c': {
                cMotion();
                editor.savedCommandSeq = editor.commandSeq;
                editor.commandSeq = "";
                break;
            }
            // Delete Command
            case 'r': {
                char c = getch();
                r(c, editor.lastMult);
                editor.savedCommandSeq = editor.commandSeq;
                editor.commandSeq = "";
                break;
            }
            // Delete Command (might need to pass in the number of times to execute the command)
            case 'x': {
                editor.lastCommand = "x";
                int charsRemaining = (int)editor.textBuffer[editor.cursorY].size() - editor.cursorX - 1;
                callDeleteCommand(editor, false, ch, std::ranges::min(charsRemaining, numOfTimes));
                editor.savedCommandSeq = editor.commandSeq;
                editor.commandSeq = "";
                break;
            }
            // Delete Command
            case 'X': {
                editor.lastCommand = "X";
                callDeleteCommand(editor, true, ch, std::ranges::min(editor.cursorX, numOfTimes));
                editor.savedCommandSeq = editor.commandSeq;
                editor.commandSeq = "";
                break;
            }
            // Delete Command
            case 's': {
                editor.clipboardContent.clear();
                std::string deletedString = "", currentLine = editor.textBuffer[editor.cursorY];
                for (int cnt = 0; cnt < editor.lastMult && editor.cursorX + cnt < (int)currentLine.size(); cnt++) {
                    deletedString += editor.textBuffer[editor.cursorY][editor.cursorX + cnt];
                }
                editor.clipboardContent.push_back(deletedString);
                callDeleteCommand(editor, false, ch, editor.lastMult);
                editor.lastCommand = "s";
                editor.savedCommandSeq = editor.commandSeq;
                editor.commandSeq = "";
                break;
            }
            // Delete Command
            case 'S': {
                cc(1);
                editor.lastCommand = "S";
                editor.savedCommandSeq = editor.commandSeq;
                editor.commandSeq = "";
                break;
            }
            // Insert Command
            case 'o': {
                openLine(true);
                break;
            }
            // Insert Command
            case 'O': {
                openLine(false);
                break;
            }
            // Insert Command (replace/edit)  (doesn't count for .)
            case 'R': {
                R();
                break;
            }
            // Search Command
            case 'f': { 
                char c = getch();
                repeatChar = c;
                upper = 0;
                findCharOnLine(true, c);
                break;
            }
            // Search Command
            case 'F': { 
                char c = getch();
                repeatChar = c;
                upper = 1;
                findCharOnLine(false, c);
                break;
            }
            // Search Command
            case ';': {
                if (upper == 1) {
                    findCharOnLine(false, repeatChar, editor.lastMult);
                }
                else if (upper == 0) {
                    findCharOnLine(true, repeatChar, editor.lastMult);
                }
                break;
            }
            // Search Command
            case '/': {
                dir = '/';
                callSearchCommand(editor, '/', editor.lastMult);
                break;
            }
            // Search Command
            case '?': {
                dir = '?';
                callSearchCommand(editor, '?', editor.lastMult);
                break;
            }
            case 'n': {
                SearchCommand searchCommand(editor, editor.lastMult);
                searchCommand.execute(query, dir);
                break;
            }
            case 'N': {
                SearchCommand searchCommand(editor, editor.lastMult);
                if (dir == '/') {
                    searchCommand.execute(query, '?');
                } else {
                    searchCommand.execute(query, '/');
                }
                break;
            }
            // Delete Command
            case 'd': {
                editor.clipboardContent.clear();
                dMotion();
                editor.savedCommandSeq = editor.commandSeq;
                editor.commandSeq = "";
                break;
            }
            // Movement Command
            case 'b': {
                callMotionCommand(editor, ch, numOfTimes);
                editor.commandSeq = "";
                break;
            }
            // Movement Command
            case 'w': {
                callMotionCommand(editor, ch, numOfTimes);
                editor.commandSeq = "";
                break;
            }
            // Undo Command (might have to handle all the undo stuff in a separate class)
            case 'u': {
                if (!editor.fileHistory.empty()) {
                    auto prev = editor.fileHistory.top();
                    editor.textBuffer = prev.first;
                    editor.setCursorPosition(prev.second.first, prev.second.second);
                    editor.fileHistory.pop();
                    editor.render();
                }
                break;
            }
            case '.': {
                if (!editor.replaying) {
                    editor.repeatLastChange();
                }
                break;
            }
            // Colon Command
            case ':': { // Handle colon commands
                colonCommand();
                editor.commandSeq = "";
                break;
            }
            // Movement Command
            case '^': {
                I();
                editor.switchMode(Mode::Command);
                editor.commandSeq = "";
                break;
            }
            // Movement Command
            case '$': {
                callMotionCommand(editor, ch, numOfTimes);
                editor.commandSeq = "";
                break;
            }
            // Movement Command
            case '0': {
                editor.setCursorPosition(editor.cursorY, 0);
                editor.commandSeq = "";
                break;
            }
            // Movement Command
            case '%': {
                percentCommand(numOfTimes);
                editor.commandSeq = "";
                break;
            }
            // Handle cursor movement
            case 'h':
                callMotionCommand(editor, ch, numOfTimes);
                break;
            case 'j':
                callMotionCommand(editor, ch, numOfTimes);
                break;
            case 'k':
                callMotionCommand(editor, ch, numOfTimes);
                break;
            case 'l':
                callMotionCommand(editor, ch, numOfTimes);
                break;
            default:
                //editor.setStatusMessage("");
                break;
        }
        // Control Commands
        // Movement Command
        if (ch == ('b' & 0x1F)) {
            callMotionCommand(editor, ch, numOfTimes);
        }
        // Movement Command
        else if (ch == ('d' & 0x1F)) {
            callMotionCommand(editor, ch, numOfTimes);
        }
        // Movement Command
        else if (ch == ('u' & 0x1F)) {
            callMotionCommand(editor, ch, numOfTimes);
        }
        // Movement Command
        else if (ch == ('f' & 0x1F)) {
            callMotionCommand(editor, ch, numOfTimes);
        }
        else if (ch == ('g' & 0x1F)) {
            ctrlG();
        }
        else if (ch == ('v' & 0x1F)) {
            editor.enterVisualModeBlock();
        }

        if (fun) {
            for (int cnt = 0; cnt < editor.lastMult; cnt++) {
                (this->*fun)();
            }
        }
        if (ch != 'u' && editor.fileHistory.empty()) {
            editor.fileHistory.push({editor.textBuffer, {editor.cursorY, editor.cursorX}});
        }   
        else if (ch != 'u' && !editor.fileHistory.empty() && editor.textBuffer != editor.fileHistory.top().first) {
            editor.fileHistory.push({editor.textBuffer, {editor.cursorY, editor.cursorX}});
        }
        else if (ch == 27) {
            editor.commandSeq = "";
        }
    } else if (mode == Mode::Insert) {
        if (!editor.replaying) 
            editor.commandSeq += (char)ch;
        editor.setStatusMessage("");
        if (ch == 27) { // ESC key
            if (!editor.replaying)
                editor.recordLastChangeKeys(editor.commandSeq);
            editor.commandSeq = "";
            editor.switchMode(Mode::Command);
            editor.moveCursor(0, -1);
            if (editor.lastCommand != "s" && editor.lastCommand != "S")
                editor.handleInsertToCommand();
        } 
        else if (isprint(ch)) {
            // Create an InsertCommand
            return std::make_shared<InsertCommand>(editor, static_cast<char>(ch));
        } else if (ch == '\n' || ch == KEY_ENTER) {
            // Handle new line insertion 
            return std::make_shared<InsertCommand>(editor, '\n');
        } 
        else if (ch == KEY_DC) {
            return std::make_shared<DeleteCommand>(editor, false, ch, 1);
        }
        else if (ch == KEY_BACKSPACE) {
            // Create a DeleteCommand
            return std::make_shared<DeleteCommand>(editor, true, ch, 1);
        }
    } else if (mode == Mode::Visual) {
        if (!editor.replaying && ch != '.') {
            editor.commandSeq += (char)ch;
        }

        if (ch == 27) { // Esc
            editor.exitVisualMode();
        } else if (ch == '.' && !editor.replaying) {
            editor.repeatLastChange();
        } else if (ch == 'h' || ch == 'j' || ch == 'k' || ch == 'l') {
            // Move cursor
            // Suppose moveCursor handles bounds
            switch(ch) {
                case 'h': editor.moveCursor(0, -1); break;
                case 'j': editor.moveCursor(1, 0); break;
                case 'k': editor.moveCursor(-1, 0); break;
                case 'l': editor.moveCursor(0, 1); break;
            }
            editor.updateVisualSelection();
        } else if (ch == 'y') {
            editor.yankSelection();
        } else if (ch == 'd' || ch == 'c') {
            editor.deleteSelection();
            editor.recordLastChangeKeys(editor.commandSeq);
            if (ch == 'c')
                editor.switchMode(Mode::Insert);
        } else if (ch == 'x' || ch == 'X') {
            editor.deleteSelection();
            editor.recordLastChangeKeys(editor.commandSeq);
        }
        else if (ch == 'v') {
            editor.enterVisualModeChar();
        } else if (ch == 'V') {
            editor.enterVisualModeLine();
        } else if (ch == 22) {
            editor.enterVisualModeBlock();
        }
    }
    return nullptr;
}

void CommandParser::a() {
    editor.switchMode(Mode::Insert);
    editor.setStatusMessage("");
    editor.moveCursor(0, 1);
}

void CommandParser::A() {
    editor.switchMode(Mode::Insert);
    const std::string& currentLine = editor.textBuffer[editor.cursorY];
    editor.setCursorPosition(editor.cursorY, currentLine.size());
}

void CommandParser::cc(int numOfTimes) {
    int linesLeft = (int)editor.textBuffer.size() - editor.cursorY;
    for (int cnt = 0; cnt < std::ranges::min(linesLeft, numOfTimes * editor.lastMult); cnt++) {
        dd();
    }
    editor.insertCharacter('\n');
    editor.moveCursor(-1, 0);
    editor.lastCommand = "line";
}

void CommandParser::cMotion() {
    char ch = getch();
    int numOfTimes = 1;
    if (ch > '0' && ch <= '9') {
        numOfTimes = ch - '0';
        ch = getch();
        while (ch >= '0' && ch <= '9') {
            numOfTimes = numOfTimes * 10 + (ch - '0');
            ch = getch();
        }
    }
    editor.commandSeq += std::to_string(numOfTimes);
    editor.commandSeq += ch;

    if (ch == '0') {
        editor.lastCommand = "inline";
        callDeleteCommand(editor, true, 8, editor.cursorX);
    }
    else if (ch == '$') {
        for (int cnt = 0; cnt < numOfTimes * editor.lastMult; cnt++) {
            dd();
        }
        if (numOfTimes * editor.lastMult > 1) {
            editor.lastCommand = "line";
        }
    }
    else if (ch == 'k') {
        int curY = editor.cursorY;
        int n = std::ranges::min(curY, numOfTimes * editor.lastMult);
        for (int cnt = 0; cnt < n; cnt++) {
            dd();
            editor.cursorY--;
        }
        if (n >= 1) {
            editor.lastCommand = "line";
        }
    }
    else if (ch == 'j') {
        int linesLeft = (int)editor.textBuffer.size() - editor.cursorY;
        int n = std::ranges::min(linesLeft, numOfTimes * editor.lastMult);
        for (int cnt = 0; cnt < n; cnt++) {
            dd();
        }
        if (n >= 1) {
            editor.lastCommand = "line";
        }
    }
    else if (ch == 'h') {
        callDeleteCommand(editor, true, 8, std::ranges::min((int)editor.cursorX, numOfTimes * editor.lastMult));
        editor.lastCommand = "inline";
    }
    else if (ch == 'l') {
        std::string currentLine = editor.textBuffer[editor.cursorY];
        callDeleteCommand(editor, false, 127, std::ranges::min((int)currentLine.size() - editor.cursorX, numOfTimes * editor.lastMult));
        editor.lastCommand = "inline";
    }
    else if (ch == 'c') {
        cc(numOfTimes);
    }
    else if (ch == 'w' || ch == 'b' || ch == '%') {
        int curX = editor.cursorX, curY = editor.cursorY;
        for (int cnt = 0; cnt < numOfTimes * editor.lastMult; cnt++) {
            callMotionCommand(editor, ch, 1);
        }
        deleteRange(curY, curX, editor.cursorY, editor.cursorX);
        editor.lastCommand = "inline";
    } 
    else if (ch == 'f') {
        int curX = editor.cursorX, curY = editor.cursorY;
        char c = getch();
        repeatChar = c;
        upper = 0;
        findCharOnLine(true, c);
        deleteRange(curY, curX, editor.cursorY, editor.cursorX);
        callDeleteCommand(editor, false, 127, 1);
    }
    else if (ch == 'F') {
        int curX = editor.cursorX, curY = editor.cursorY;
        char c = getch();
        repeatChar = c;
        upper = 1;
        findCharOnLine(false, c);
        deleteRange(curY, curX, editor.cursorY, editor.cursorX);
        callDeleteCommand(editor, false, 127, 1);
    }
    else {
        return;
    }
    editor.switchMode(Mode::Insert);
}

void CommandParser::dd() {
    editor.lastCommand = "dd";
    editor.clipboardContent.clear();

    std::string deletedLine = editor.textBuffer[editor.cursorY];
    editor.setClipboard(std::vector<std::string>{deletedLine});

    editor.textBuffer.erase(editor.textBuffer.begin() + editor.cursorY);
    if (editor.textBuffer.empty()) {
        editor.textBuffer.emplace_back("");
    }

    if (editor.cursorY >= static_cast<int>(editor.textBuffer.size())) {
        editor.cursorY = editor.textBuffer.size() - 1;
    }
    editor.cursorX = 0;

    editor.setStatusMessage("Deleted line.", false);
}

void CommandParser::deleteWord() {
    return;
}

void CommandParser::deleteRange(int startY, int startX, int endY, int endX) {
    int startLineLen = (int)editor.textBuffer[startY].size();
    int endLineLen = (int)editor.textBuffer[endY].size();
    if (startX < 0) startX = 0;
    if (startX > startLineLen) startX = startLineLen;
    if (endX < 0) endX = 0;
    if (endX > endLineLen) endX = endLineLen;

    if ((endY < startY) || (endY == startY && endX < startX)) {
        std::swap(startY, endY);
        std::swap(startX, endX);
    }

    if (startY == endY) {
        std::string &line = editor.textBuffer[startY];
        if (endX >= startX && startX < (int)line.size()) {
            line.erase(startX, (endX - startX));
        }
    } else {
        std::string &startLine = editor.textBuffer[startY];
        std::string &endLine = editor.textBuffer[endY];

        if (startX < (int)startLine.size()) {
            startLine.erase(startX);
        }

        if (endX >= 0 && endX < (int)endLine.size()) {
            endLine.erase(0, endX);
        }

        if (endY - startY > 1) {
            editor.textBuffer.erase(editor.textBuffer.begin() + startY + 1, editor.textBuffer.begin() + endY);
        }

        editor.textBuffer[startY] += endLine;
        if (endY > startY) {
            editor.textBuffer.erase(editor.textBuffer.begin() + startY + 1);
        }
    }

    if (startY >= (int)editor.textBuffer.size()) {
        startY = (int)editor.textBuffer.size() - 1;
    }
    if (startY < 0) startY = 0;
    int newLineLen = (int)editor.textBuffer[startY].size();
    if (startX > newLineLen) startX = newLineLen;

    editor.setCursorPosition(startY, startX);
}


void CommandParser::dMotion() {
    char ch = getch();
    int numOfTimes = 1;
    if (ch > '0' && ch <= '9') {
        numOfTimes = ch - '0';
        ch = getch();
        while (ch >= '0' && ch <= '9') {
            numOfTimes = numOfTimes * 10 + (ch - '0');
            ch = getch();
        }
    }
    editor.commandSeq += std::to_string(numOfTimes);
    editor.commandSeq += ch;

    if (ch == '0') {
        editor.lastCommand = "inline";
        callDeleteCommand(editor, true, 8, editor.cursorX);
    }
    else if (ch == '$') {
        callDeleteCommand(editor, false, 127, editor.textBuffer[editor.cursorY].size() - editor.cursorX);
        for (int cnt = 0; cnt < numOfTimes * editor.lastMult - 1; cnt++) {
            dd();
        }
        if (numOfTimes * editor.lastMult > 1) {
            editor.lastCommand = "line";
        }
    }
    else if (ch == 'k') {
        int curY = editor.cursorY;
        int n = std::ranges::min(curY, numOfTimes * editor.lastMult + 1);
        for (int cnt = 0; cnt < n; cnt++) {
            dd();
            editor.cursorY--;
        }
        if (n >= 1) {
            editor.lastCommand = "line";
        }
    }
    else if (ch == 'j') {
        int linesLeft = (int)editor.textBuffer.size() - editor.cursorY;
        int n = std::ranges::min(linesLeft, numOfTimes * editor.lastMult + 1);
        for (int cnt = 0; cnt < n; cnt++) {
            dd();
        }
        if (n >= 1) {
            editor.lastCommand = "line";
        }
    }
    else if (ch == 'h') {
        callDeleteCommand(editor, true, 8, std::ranges::min((int)editor.cursorX, numOfTimes * editor.lastMult));
        editor.lastCommand = "inline";
    }
    else if (ch == 'l') {
        std::string currentLine = editor.textBuffer[editor.cursorY];
        callDeleteCommand(editor, false, 127, std::ranges::min((int)currentLine.size() - editor.cursorX, numOfTimes * editor.lastMult));
        editor.lastCommand = "inline";
    }
    else if (ch == 'd') {
        int linesLeft = (int)editor.textBuffer.size() - editor.cursorY;
        for (int cnt = 0; cnt < std::ranges::min(linesLeft, numOfTimes * editor.lastMult); cnt++) {
            dd();
        }
        editor.lastCommand = "line";
    }
    else if (ch == 'w' || ch == 'b' || ch == '%') {
        int curX = editor.cursorX, curY = editor.cursorY;
        for (int cnt = 0; cnt < numOfTimes * editor.lastMult; cnt++) {
            callMotionCommand(editor, ch, 1);
        }
        deleteRange(curY, curX, editor.cursorY, editor.cursorX);
        editor.lastCommand = "inline";
    }
    else if (ch == 'f') {
        int curX = editor.cursorX, curY = editor.cursorY;
        char c = getch();
        repeatChar = c;
        upper = 0;
        findCharOnLine(true, c);
        deleteRange(curY, curX, editor.cursorY, editor.cursorX);
    }
    else if (ch == 'F') {
        int curX = editor.cursorX, curY = editor.cursorY;
        char c = getch();
        repeatChar = c;
        upper = 1;
        findCharOnLine(false, c);
        deleteRange(curY, curX, editor.cursorY, editor.cursorX);
    }
    else {
        return;
    }
}

void CommandParser::findCharOnLine(bool lower, char c, int mult) {
    int curX = editor.cursorX;
    bool flag = true;
    int multiplier = 1;
    if (mult > 0) {
        multiplier = mult;
    } else {
        multiplier = editor.lastMult;
        editor.backupMult = editor.lastMult;
    }
    for (int cnt = 0; cnt < multiplier; cnt++) {
        const std::string& currentLine = lower ? editor.textBuffer[editor.cursorY] : editor.textBuffer[editor.cursorY].substr(0, editor.cursorX);
        size_t pos = lower ? currentLine.find(c, editor.cursorX + 1) : currentLine.rfind(c);

        if (pos != std::string::npos) {
            editor.setCursorPosition(editor.cursorY, pos);
            editor.setStatusMessage(std::string("Found '") + c + "' at position " + std::to_string(pos), false);
        } else {
            flag = false;
            editor.setStatusMessage(std::string("Character '") + c + "' not found", true);
        }
    }
    if (!flag) {
        editor.setCursorPosition(editor.cursorY, curX);
        beep(); 
    }
}

void CommandParser::i() {
    editor.setStatusMessage("");
    editor.switchMode(Mode::Insert);
}

void CommandParser::I() {
    const std::string& currentLine = editor.textBuffer[editor.cursorY];
    size_t pos = currentLine.find_first_not_of(" \t");
    
    if (pos != std::string::npos) {
        editor.setCursorPosition(editor.cursorY, static_cast<int>(pos));
        editor.setStatusMessage("Moved to first non-blank character.", false);
    }
    else {
        editor.setCursorPosition(editor.cursorY, currentLine.size());
        editor.setStatusMessage("Line is empty or contains only whitespace.", true);
    }
    
    editor.switchMode(Mode::Insert);
}

void CommandParser::J() {
    if (editor.cursorY + 1 >= (int)editor.textBuffer.size()) {
        beep();
        return;
    }
    std::string currentLine = editor.textBuffer[editor.cursorY];
    std::string nextLine = editor.textBuffer[editor.cursorY + 1];

    size_t start = nextLine.find_first_not_of(" \t");
    if (start != std::string::npos)
        nextLine = nextLine.substr(start);

    int newCursorX = currentLine.length();

    // Merge lines
    bool needSpace = false;
    size_t end = currentLine.find_last_not_of(" \t");
    if (end == std::string::npos || end == currentLine.length() - 1) {
        needSpace = true;
    }
    if (needSpace) {
        currentLine += " " + nextLine;
    } else {
        currentLine += nextLine;
    }
    editor.textBuffer[editor.cursorY] = currentLine;
    editor.textBuffer.erase(editor.textBuffer.begin() + editor.cursorY + 1);

    editor.setCursorPosition(editor.cursorY, newCursorX);
    editor.preferredCursorX = newCursorX;

    editor.setStatusMessage("Joined lines.", false);
}

void CommandParser::openLine(bool lower) {
    int insertPosition = editor.cursorY;
    if (lower) {
        insertPosition++;
    }
    editor.input.push_back('\n');
    editor.textBuffer.insert(editor.textBuffer.begin() + insertPosition, "");
    if (lower) {
        editor.cursorY++;
    }
    editor.setCursorPosition(editor.cursorY, 0);
    editor.switchMode(Mode::Insert);
}

void CommandParser::paste(bool lower) {
    if (editor.clipboardContent.empty()) {
        editor.setStatusMessage("Clipboard is empty. Nothing to paste.", true);
        beep();
        return;
    }
    int insertPosition = editor.cursorY;
    if (lower) {
        insertPosition++;
    }

    editor.fileHistory.push({editor.textBuffer, {editor.cursorY, editor.cursorX}});
    editor.textBuffer.insert(editor.textBuffer.begin() + insertPosition, editor.clipboardContent.begin(), editor.clipboardContent.end());

    if (lower) {
        editor.cursorY += editor.clipboardContent.size();
    }
    editor.setCursorPosition(editor.cursorY, 0); 

}

void CommandParser::r(char c, int numOfTimes) {
    if (editor.cursorX + numOfTimes > static_cast<int>(editor.textBuffer[editor.cursorY].size())) {
        beep();
        return;
    }
    for (int cnt = 0; cnt < numOfTimes; cnt++) {
        editor.deleteCharacter(false);
        editor.insertCharacter(c);
    }
    editor.cursorX -= 1;
}

void CommandParser::R() {
    editor.setStatusMessage("");
    editor.switchMode(Mode::Replace);
    editor.render();
    editor.input.clear();
    char c = getch();
    while (c != 27) {
        if (editor.cursorX < static_cast<int>(editor.textBuffer[editor.cursorY].size())) {
            r(c, 1);
            editor.cursorX += 1;
        }
        else {
            editor.insertCharacter(c);
        }
        editor.render();
        c = getch();
    }
    editor.handleInsertToCommand();
    editor.switchMode(Mode::Command);
}

void CommandParser::copyCharacters(Editor &editor, char ch, int count) {
    int y = editor.getCursorY();
    int x = editor.getCursorX();
    std::string &line = editor.textBuffer[y];

    int length;
    if (ch == 'h') { 
        // 'h' motion (backwards), copy count chars backwards
        length = std::min(x, count);
        int start = x - length + 1;
        std::string extracted = line.substr(start, length);
        editor.clipboardContent.push_back(extracted);
    }
    else if (ch == 'l') {
        // 'l' motion (forwards), copy count chars forwards
        int remaining = (int)line.size() - x;
        length = std::min(remaining, count);
        std::string extracted = line.substr(x, length);
        editor.clipboardContent.push_back(extracted);
    }
}

void CommandParser::copyWord(int multiplier, int direction) {
    // direction: +1 for forward (yw), -1 for backward (yb)
    // multiplier: number of words to yank

    std::string &line = editor.textBuffer[editor.cursorY];
    std::string yankedData;

    int originalY = editor.cursorY;
    int originalX = editor.cursorX;

    for (int i = 0; i < multiplier; i++) {
        if (direction > 0) {
            // Forward (yw)
            int startPos = editor.cursorX;
            while (startPos < (int)line.size() && !std::isalnum((unsigned char)line[startPos])) {
                startPos++;
            }
            int endPos = startPos;
            while (endPos < (int)line.size() && std::isalnum((unsigned char)line[endPos])) {
                endPos++;
            }

            int yankStart = editor.cursorX;
            int yankEnd = endPos;
            if (yankEnd > (int)line.size()) yankEnd = (int)line.size();

            if (yankStart < yankEnd) {
                yankedData.append(line.substr(yankStart, yankEnd - yankStart));
            }

            editor.cursorX = endPos; 
        } else {
            // Backward (yb)
            int startPos = editor.cursorX - 1;
            if (startPos < 0) startPos = 0;
            while (startPos >= 0 && !std::isalnum((unsigned char)line[startPos])) {
                startPos--;
            }
            int endPos = startPos;
            while (endPos >= 0 && std::isalnum((unsigned char)line[endPos])) {
                endPos--;
            }

            int yankStart = endPos + 1;
            int yankEnd = editor.cursorX; 
            if (yankStart < 0) yankStart = 0;
            if (yankStart < yankEnd) {
                yankedData.append(line.substr(yankStart, yankEnd - yankStart));
            }

            editor.cursorX = yankStart; 
        }
    }

    editor.cursorY = originalY;
    editor.cursorX = originalX;

    editor.clipboardContent.clear();
    editor.clipboardContent.push_back(yankedData);

    editor.setStatusMessage("Yanked word(s)", false);
}

void CommandParser::copyLines(Editor &editor, int count) {
    // dd but just copy lines into clipboard without removing
    int startY = editor.getCursorY();
    int endY = startY + (count - 1);
    if (endY >= (int)editor.textBuffer.size()) {
        endY = (int)editor.textBuffer.size() - 1;
    }

    std::vector<std::string> copiedLines;
    for (int y = startY; y <= endY; y++) {
        copiedLines.push_back(editor.textBuffer[y]);
    }
    editor.setClipboard(copiedLines);
    editor.setStatusMessage("Yanked " + std::to_string(copiedLines.size()) + " line(s).", false);
}


void CommandParser::yMotion() {
    char ch = getch();
    int numOfTimes = 1;
    if (ch > '0' && ch <= '9') {
        numOfTimes = ch - '0';
        ch = getch();
        while (ch >= '0' && ch <= '9') {
            numOfTimes = numOfTimes * 10 + (ch - '0');
            ch = getch();
        }
    }

    editor.commandSeq += std::to_string(numOfTimes);
    editor.commandSeq += ch;

    if (ch == '0') {
        int y = editor.getCursorY();
        int x = editor.getCursorX();
        std::string &line = editor.textBuffer[y];
        // This would copy from start of line (0) to x
        std::string extracted = line.substr(0, x);
        editor.setClipboard(std::vector<std::string>{extracted});
        editor.lastCommand = "inline";
    }
    else if (ch == '$') {
        int startY = editor.getCursorY();
        int startX = editor.getCursorX();
        int multiplier = editor.lastMult * numOfTimes;
        int endY = startY + (multiplier - 1);
        if (endY >= (int)editor.textBuffer.size()) {
            endY = (int)editor.textBuffer.size() - 1;
        }

        std::vector<std::string> copiedLines;

        // If just y$, multiplier=1, copy from cursorX to end of current line
        if (multiplier == 1) {
            std::string &line = editor.textBuffer[startY];
            std::string extracted = (startX < (int)line.size()) ? line.substr(startX) : "";
            copiedLines.push_back(extracted);
            editor.lastCommand = "inline";
        } else {
            // For multiple lines
            // First line: from cursorX to end
            std::string firstLinePart = editor.textBuffer[startY].substr(startX);
            copiedLines.push_back(firstLinePart);

            // Intermediate lines: copy them entirely
            for (int l = startY + 1; l < endY; l++) {
                copiedLines.push_back(editor.textBuffer[l]);
            }

            // Last line (endY > startY): copy entire line up to endX (which is line length)
            if (endY > startY) {
                copiedLines.push_back(editor.textBuffer[endY]);
            }
            editor.lastCommand = "line";
        }
        editor.setClipboard(copiedLines);
    }
    else if (ch == 'k') {
        int linesBefore = std::ranges::min(editor.cursorY, editor.lastMult * numOfTimes);
        if (linesBefore )
        editor.cursorY -= linesBefore;
        copyLines(editor, linesBefore);
        editor.cursorY += linesBefore;
        editor.lastCommand = "line";
    }
    else if (ch == 'j') {
        int linesLeft = (int)editor.textBuffer.size() - editor.cursorY;
        int linesToCopy = std::min(linesLeft, numOfTimes * editor.lastMult);
        copyLines(editor, linesToCopy);
        editor.lastCommand = "line";
    }
    else if (ch == 'h') {
        int available = editor.cursorX;
        int toCopy = std::min(available, numOfTimes * editor.lastMult);
        copyCharacters(editor, ch, toCopy);
        editor.lastCommand = "inline";
    }
    else if (ch == 'l') {
        std::string currentLine = editor.textBuffer[editor.cursorY];
        int available = (int)currentLine.size() - editor.cursorX;
        int toCopy = std::min(available, numOfTimes * editor.lastMult);
        copyCharacters(editor, ch, toCopy);
        editor.lastCommand = "inline";
    }
    else if (ch == 'y') {
        int linesLeft = (int)editor.textBuffer.size() - editor.cursorY;
        int linesToCopy = std::min(linesLeft, numOfTimes * editor.lastMult);
        copyLines(editor, linesToCopy);
        editor.lastCommand = "line";
    }
    else if (ch == 'w') {
        copyWord(numOfTimes * editor.lastMult, 1); 
        editor.lastCommand = "inline";
    }
    else if (ch == 'b') {
        copyWord(numOfTimes * editor.lastMult, -1);
        editor.lastCommand = "inline";
    } else {
        return;
    }
}

void CommandParser::ctrlG() {
    std::string fileInfo;
    std::string fileName = editor.getFilename();

    if (!fileName.empty()) {
        fileInfo += "File: " + fileName + " ";
    } else {
        fileInfo += "No Name ";
    }

    bool modified = editor.getIsModified();
    std::string modStatus = modified ? " [Modified] " : "";
    fileInfo += modStatus;

    int totalLines = editor.textBuffer.size();
    fileInfo += std::to_string(totalLines) + " lines ";

    int currentLine = editor.cursorY + 1; 
    int currentCol = editor.cursorX + 1;
    fileInfo += std::to_string(currentLine) + "," + std::to_string(currentCol) + " ";

    double percentage = (double)currentLine / totalLines;
    fileInfo += " --" + std::to_string(std::ceil(percentage * 100.0) / 100.0) + "%--";

    editor.setStatusMessage(fileInfo, false);
}

void CommandParser::percentCommand(int percentage) {
    if (percentage > 0 && percentage <= 100) {
        int lineNumber = ((double)percentage / 100) * editor.getMaxCursorY();
        editor.setCursorPosition(lineNumber, 0);
        while (editor.textBuffer[editor.cursorY][editor.cursorX] == ' ') {
            editor.cursorX++;
        }
        return;
    }
    if (percentage == -1) {
        // user did not enter any number, jump to matching brace based on behaviour
        // if the cursor is not currently on some bracket 
        // --> move to the next closing bracket (if exists on current line), 
        // --> else move to the corresponding closing bracket of the next opening bracket (if exists on current line)
        // else beep();
        // if the cursor is currently on some opening bracket --> go until you find the matching closing bracket, else beep();
        // if the cursor is currently on some closing bracket --> go back until you find the matching opening bracket, else beep();
        char currentChar = editor.textBuffer[editor.cursorY][editor.cursorX];
        if (currentChar != '{' && currentChar != '[' && currentChar != '(' && currentChar != '}' && currentChar != ']' && currentChar != ')') {
            int curX = editor.cursorX;
            bool flag = false;
            while (curX < static_cast<int>(editor.textBuffer[editor.cursorY].size())) {
                char checkClosing = editor.textBuffer[editor.cursorY][++curX];
                if (checkClosing == '}' || checkClosing == ']' || checkClosing == ')') {
                    flag = true;
                    break;
                }
            }
            if (flag) {
                editor.setCursorPosition(editor.cursorY, curX);
                return;
            }
            curX = editor.cursorX;
            flag = false;
            while (curX < static_cast<int>(editor.textBuffer[editor.cursorY].size())) {
                char checkOpening = editor.textBuffer[editor.cursorY][++curX];
                if (checkOpening == '{' || checkOpening == '[' || checkOpening == '(') {
                    flag = true;
                    break;
                }
            }
            if (flag) {
                findMatchingBracket(curX, editor.cursorY, true);
            }

        }
        else if (currentChar == '{' || currentChar == '(' || currentChar == '[') {
            findMatchingBracket(editor.cursorX, editor.cursorY, true);
        }
        else {
            findMatchingBracket(editor.cursorX, editor.cursorY, false);
        }
    }
}

void CommandParser::findMatchingBracket(int startX, int startY, bool dir) {

    std::stack<char> stk;
    char bracketType = editor.textBuffer[startY][startX];
    stk.push(bracketType);
    if (dir) {
        for (int y = startY; y < static_cast<int>(editor.textBuffer.size()); ++y) {
            int xStart = (y == startY) ? startX + 1 : 0;
            for (int x = xStart; x < static_cast<int>(editor.textBuffer[y].size()); ++x) {
                char currentChar = editor.textBuffer[y][x];
                if (currentChar == bracketType) {
                    stk.push(currentChar);
                }
                else if ((bracketType == '{' && currentChar == '}') || (bracketType == '(' && currentChar == ')') || (bracketType == '[' && currentChar == ']')) {
                    stk.pop();
                    if (stk.empty()) {
                        editor.setCursorPosition(y, x);
                        return;
                    }
                }
            }
        }
    } else {
        for (int y = startY; y >= 0; --y) {
            int xEnd = (y == startY) ? startX - 1 : editor.textBuffer[y].size() - 1;
            for (int x = xEnd; x >= 0; --x) {
                char currentChar = editor.textBuffer[y][x];
                if (currentChar == bracketType) {
                    stk.push(currentChar);
                }
                else if ((bracketType == '}' && currentChar == '{') || (bracketType == ')' && currentChar == '(') || (bracketType == ']' && currentChar == '[')) {
                    stk.pop();
                    if (stk.empty()) {
                        editor.setCursorPosition(y, x);
                        return;
                    }
                }
            }
        }
    }
}

void CommandParser::colonCommand() {
    std::string commandStr;
    echo(); 
    curs_set(1); 
    move(LINES - 1, 0);
    clrtoeol(); 
    printw(":"); 
    char inputBuffer[256];
    getnstr(inputBuffer, 255);
    commandStr = std::string(inputBuffer);
    noecho();
    move(LINES - 1, 0);
    clrtoeol();

    ColonCommands colonCommands(editor);
    colonCommands.execute(commandStr);
    editor.commandSeq = "";
}

void CommandParser::callMotionCommand(Editor& editor, char ch, int numOfTimes) {
    if (numOfTimes < 0)
        numOfTimes = 1;
    MotionCommand motionCommand(editor, ch, numOfTimes);
    motionCommand.execute();
    editor.commandSeq = "";
}

void CommandParser::callDeleteCommand(Editor& editor, bool type, char ch, int numOfTimes) {
    if (numOfTimes < 0)
        numOfTimes = 1;
    DeleteCommand deleteCommand(editor, type, ch, numOfTimes);
    deleteCommand.execute();
}

void CommandParser::callSearchCommand(Editor& editor, char direction, int numOfTimes) {
    std::string wordStr;
    echo(); 
    curs_set(1); 
    move(LINES - 1, 0);
    clrtoeol(); 
    if (direction == '/') {
        printw("/");
    }
    else {
        printw("?");
    }
    char inputBuffer[256];
    getnstr(inputBuffer, 255); 
    wordStr = std::string(inputBuffer);
    query = wordStr;
    noecho();
    move(LINES - 1, 0);
    clrtoeol();
    SearchCommand searchCommand(editor, numOfTimes);
    searchCommand.execute(wordStr, direction);
    editor.commandSeq = "";
}

void CommandParser::tempFun() {
    return;
}