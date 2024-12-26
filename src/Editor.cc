#include "Editor.h"
#include "CommandParser.h"
#include "ICommand.h"
#include "InsertCommand.h"
#include "DeleteCommand.h"
#include "ColonCommands.h" 
#include <ncurses.h>
#include <cctype> 
#include <memory>
#include <iostream>
#include <cassert>
#include <unordered_map>

Editor::Editor(const std::string& initFilename)
    : cursorY(0),
      cursorX(0),
      preferredCursorX(0),
      mode(Mode::Command),
      isRunning(true),
      filename(initFilename),
      viewOffsetY(0)
{
    display = std::make_shared<Display>();
    fileManager = std::make_shared<FileManager>(); 
    statusBar = std::make_unique<StatusBar>();     

    if (!filename.empty() && fileManager->fileExists(filename)) {
        if (!fileManager->loadFile(filename, textBuffer)) {
            std::cerr << "Failed to load file. Starting with an empty buffer.\n";
            textBuffer.emplace_back("");
        }
    } else {
        textBuffer.emplace_back("");
    }
    unmodifiedFile = textBuffer;
    lastMult = 1;
    replaying = false;
}

Editor::~Editor() {
}

int Editor::getCursorY() const {
    return cursorY;
}

int Editor::getCursorX() const {
    return cursorX;
}

void Editor::setCursorPosition(int y, int x) {
    cursorY = y;
    cursorX = x;
    preferredCursorX = x;
 
    ensureCursorInBounds(false);
}

void Editor::handleInsertToCommand() {
    backup = input;
    backupMult = lastMult;
    if (backup.size() == 0) {
        fileHistory.pop();
    }
    for (int i = 0; i < lastMult - 1; i++) {
        for (int j = 0; j < static_cast<int>(backup.size()); j++) {
            insertCharacter(backup[j]);
        }
    }
    input.clear();
}

void Editor::insertCharacter(int ch) {
    // If cursorY is beyond textBuffer, create new lines
    while (cursorY >= static_cast<int>(textBuffer.size())) {
        textBuffer.emplace_back("");
    }

    if (ch == '\n') {
        std::string& currentLine = textBuffer[cursorY];
        std::string newLine = currentLine.substr(cursorX);
        currentLine.erase(cursorX);
        textBuffer.insert(textBuffer.begin() + cursorY + 1, newLine);
        setCursorPosition(cursorY + 1, 0);
    } else {
        std::string& currentLine = textBuffer[cursorY];
        currentLine.insert(cursorX, 1, static_cast<char>(ch));
        setCursorPosition(cursorY, cursorX + 1);
    }
    input.push_back(static_cast<char>(ch));
}

void Editor::deleteCharacter(bool type) {
    if (textBuffer.empty()) {
        beep();
        return;
    }
    
    if (type) { // Backspace
        if (!input.empty()) {
            input.pop_back();
        }
        std::string& currentLine = textBuffer[cursorY];
        if (cursorX > 0) {
            currentLine.erase(cursorX - 1, 1);
            cursorX--;
        }
        else if (cursorY > 0) {
            std::string previousLine = textBuffer[cursorY - 1];
            int previousLineLength = previousLine.length();
            textBuffer[cursorY - 1] += currentLine;
            textBuffer.erase(textBuffer.begin() + cursorY);
            cursorY--;
            cursorX = previousLineLength;
        }
        else {
            beep();
        }
    }
    else { 
        std::string& currentLine = textBuffer[cursorY];
        if (cursorX < static_cast<int>(currentLine.length())) {
            currentLine.erase(cursorX, 1);
        }
        else if (cursorY < static_cast<int>(textBuffer.size()) - 1) {
            std::string nextLine = textBuffer[cursorY + 1];
            textBuffer[cursorY] += nextLine;
            textBuffer.erase(textBuffer.begin() + cursorY + 1);
        }
        else {
            beep();
        }
    }

    ensureCursorInBounds(false);
}

char Editor::getCharacterAtCursor() const {
    const std::string& currentLine = textBuffer[cursorY];
    if (cursorX > 0 &&
        cursorX <= static_cast<int>(currentLine.length())) {
        return currentLine[cursorX - 1];
    }
    return '\0';
}

void Editor::executeCommand(std::shared_ptr<ICommand> command) {
    if (command) {
        command->execute();
    }
}

Mode Editor::getMode() const {
    return mode;
}

void Editor::switchMode(Mode newMode) {
    if (newMode == Mode::Insert) {
        if (!fileHistory.empty() && textBuffer != fileHistory.top().first) {
            fileHistory.push({textBuffer, {cursorY, cursorX}});
        }
    }
    mode = newMode;
}

void Editor::setFilename(const std::string& fname) {
    filename = fname;
}

std::string Editor::getFilename() const {
    return filename;
}

bool Editor::loadFile(const std::string& filename) {
    if (fileManager->fileExists(filename)) {
        if (fileManager->loadFile(filename, textBuffer)) {
            setFilename(filename);
            cursorY = 0;
            cursorX = 0;
            setStatusMessage("File loaded: " + filename, false);
            return true;
        }
        setStatusMessage("Error loading file: " + filename, true);
        return false;
    } else {
        textBuffer.clear();
        textBuffer.emplace_back("");
        setFilename(filename);
        cursorY = 0;
        cursorX = 0;
        setStatusMessage("New file: " + filename, false);
        return true; 
    }
}

bool Editor::saveFile(const std::string& filename) {
    if (fileManager->saveFile(filename, textBuffer)) {
        setFilename(filename);
        setStatusMessage("File saved: " + filename, false);
        return true;
    }
    setStatusMessage("Error saving file: " + filename, true);
    return false;
}

void Editor::setIsRunning(bool running) {
    isRunning = running;
}

bool Editor::getIsModified() const {
    return unmodifiedFile == textBuffer;
}

void Editor::setStatusMessage(const std::string& message, bool isError) {
    statusBar->setStatusMessage(message, isError);
}

void Editor::render() {
    display->clear();

    int rows, cols;
    display->getWindowSize(rows, cols);

    int endLine = viewOffsetY + (rows - 1);
    if (endLine > (int)textBuffer.size()) {
        endLine = (int)textBuffer.size();
    }

    bool hasSelection = (mode == Mode::Visual && visualModeHandler.hasSelection());
    int selStartY = 0, selStartX = 0, selEndY = 0, selEndX = 0;
    VisualType vType = VisualType::Character;
    if (hasSelection) {
        visualModeHandler.getSelectionBounds(selStartY, selStartX, selEndY, selEndX);
        vType = visualModeHandler.getVisualType();
    }

    WINDOW* win = display->getWindow();
    werase(win);
    leaveok(win, FALSE);

    int visibleRows = rows - 1; // last line for status bar
    bool isCppFile = (filename.ends_with(".h") || filename.ends_with(".cc"));
    if (!enhanced) {
        
    }

    for (int i = 0; i < visibleRows; i++) {
        int actualLine = viewOffsetY + i;
        std::string line;
        if (actualLine < (int)textBuffer.size()) {
            line = textBuffer[actualLine];
        } else {
            line.clear();
        }

        bool isLineSelected = false;
        if (hasSelection && vType == VisualType::Line) {
            if (actualLine >= selStartY && actualLine <= selEndY) {
                isLineSelected = true;
            }
        }

        int blockStartX = 0, blockEndX = -1;
        if (hasSelection && vType == VisualType::Block) {
            blockStartX = std::min(selStartX, selEndX);
            blockEndX = std::max(selStartX, selEndX);
        }

        if (isCppFile && actualLine < (int)textBuffer.size()) {
            auto highlightedTokens = display->syntaxHighlighter.highlight(line);

            int x = 0;
            for (const auto& tokenPair : highlightedTokens) {
                TokenType type = tokenPair.first;
                const std::string& text = tokenPair.second;

                int colorPairToUse = 2;
                attr_t attrToUse = A_NORMAL;
                switch (type) {
                    case TokenType::Keyword: colorPairToUse = 6; break;
                    case TokenType::NumericLiteral: colorPairToUse = 7; break;
                    case TokenType::StringLiteral: colorPairToUse = 8; break;
                    case TokenType::Identifier: colorPairToUse = 9; break;
                    case TokenType::Comment: colorPairToUse = 10; break;
                    case TokenType::PreprocessorDirective: colorPairToUse = 11; break;
                    case TokenType::Operator:
                    case TokenType::Punctuation:
                        colorPairToUse = 12;
                        break;
                    case TokenType::MismatchedBrace:
                    case TokenType::MismatchedBracket:
                    case TokenType::MismatchedParenthesis:
                        colorPairToUse = 13; attrToUse = A_BOLD;
                        break;
                    case TokenType::PlainText:
                    default:
                        colorPairToUse = 2; break;
                }

                for (size_t charIdx = 0; charIdx < text.size() && x < cols; charIdx++, x++) {
                    bool charInSelection = false;
                    if (hasSelection) {
                        if (vType == VisualType::Character) {
                            if ((actualLine > selStartY || (actualLine == selStartY && (int)x >= selStartX)) &&
                                (actualLine < selEndY || (actualLine == selEndY && (int)x <= selEndX))) {
                                charInSelection = true;
                            }
                        } else if (vType == VisualType::Line) {
                            charInSelection = isLineSelected; // entire line selected
                        } else if (vType == VisualType::Block) {
                            if (actualLine >= selStartY && actualLine <= selEndY &&
                                x >= blockStartX && x <= blockEndX) {
                                charInSelection = true;
                            }
                        }
                    }

                    if (charInSelection) {
                        wattron(win, A_REVERSE);
                        mvwaddch(win, i, x, text[charIdx]);
                        wattroff(win, A_REVERSE);
                    } else {
                        wattron(win, COLOR_PAIR(colorPairToUse) | attrToUse);
                        mvwaddch(win, i, x, text[charIdx]);
                        wattroff(win, COLOR_PAIR(colorPairToUse) | attrToUse);
                    }
                }
            }

            int lineLen = (int)line.size();
            for (int xPos = lineLen; xPos < cols; xPos++) {
                bool charInSelection = false;
                if (hasSelection && vType == VisualType::Line && isLineSelected) {
                    charInSelection = true; 
                }
                if (charInSelection) {
                    wattron(win, A_REVERSE);
                    mvwaddch(win, i, xPos, ' ');
                    wattroff(win, A_REVERSE);
                } else {
                    wattron(win, COLOR_PAIR(5));
                    mvwaddch(win, i, xPos, ' ');
                    wattroff(win, COLOR_PAIR(5));
                }
            }

        } else {
            int lineLen = (int)line.size();
            for (int x = 0; x < lineLen && x < cols; x++) {
                bool charInSelection = false;
                if (hasSelection) {
                    if (vType == VisualType::Character) {
                        if ((actualLine > selStartY || (actualLine == selStartY && x >= selStartX)) &&
                            (actualLine < selEndY || (actualLine == selEndY && x <= selEndX))) {
                            charInSelection = true;
                        }
                    } else if (vType == VisualType::Line) {
                        charInSelection = isLineSelected;
                    } else if (vType == VisualType::Block) {
                        if (actualLine >= selStartY && actualLine <= selEndY &&
                            x >= blockStartX && x <= blockEndX) {
                            charInSelection = true;
                        }
                    }
                }

                if (charInSelection) {
                    wattron(win, A_REVERSE);
                    mvwaddch(win, i, x, line[x]);
                    wattroff(win, A_REVERSE);
                } else {
                    wattron(win, COLOR_PAIR(2));
                    mvwaddch(win, i, x, line[x]);
                    wattroff(win, COLOR_PAIR(2));
                }
            }

            for (int x = lineLen; x < cols; x++) {
                bool charInSelection = false;
                if (hasSelection && vType == VisualType::Line && isLineSelected) {
                    charInSelection = true;
                }
                if (charInSelection) {
                    wattron(win, A_REVERSE);
                    mvwaddch(win, i, x, ' ');
                    wattroff(win, A_REVERSE);
                } else {
                    wattron(win, COLOR_PAIR(5));
                    mvwaddch(win, i, x, ' ');
                    wattroff(win, COLOR_PAIR(5));
                }
            }
        }

        if (actualLine >= (int)textBuffer.size()) {
            if (actualLine >= (int)textBuffer.size()) {
                wattron(win, COLOR_PAIR(5));
                mvwprintw(win, i, 0, "~");
                wattroff(win, COLOR_PAIR(5));
            }
        }
    }

    if (mode == Mode::Visual) {
        VisualType vt = visualModeHandler.getVisualType();
        if (vt == VisualType::Character) {
            statusBar->setLeftText("-- VISUAL --");
        } else if (vt == VisualType::Line) {
            statusBar->setLeftText("-- VISUAL LINE --");
        } else if (vt == VisualType::Block) {
            statusBar->setLeftText("-- VISUAL BLOCK --");
        }
    } else if (mode == Mode::Insert) {
        statusBar->setLeftText("-- INSERT --");
    } else if (mode == Mode::Replace) {
        statusBar->setLeftText("-- REPLACE --");
    } else {
        statusBar->setLeftText(!filename.empty() ? filename : "-- COMMAND --");
    }

    std::string right = "Line: " + std::to_string(cursorY + 1) +
                        ", Col: " + std::to_string(cursorX + 1);
    statusBar->setRightText(right);
    statusBar->render(win, rows, cols);

    int relativeCursorY = cursorY - viewOffsetY;
    if (relativeCursorY >= 0 && relativeCursorY < rows - 1) {
        wmove(win, relativeCursorY, cursorX);
    }

    curs_set(1);
    wrefresh(win);
}

void Editor::handleInput(int ch, CommandParser& commandParser) {
    auto command = commandParser.parseInput(ch);

    if (command) {
        executeCommand(command);
    }
    
}

void Editor::moveCursor(int deltaY, int deltaX) {
    cursorY += deltaY;

    if (deltaX != 0) {
        cursorX += deltaX;
        preferredCursorX = cursorX; 
    }

    ensureCursorInBounds(deltaY != 0); 
}

void Editor::ensureCursorInBounds(bool vertical) {
    int maxCursorY = getMaxCursorY();

    int rows, cols;
    display->getWindowSize(rows, cols);
    if (cursorY < 0) {
        cursorY = 0;
    } else if (cursorY > maxCursorY) {
        cursorY = maxCursorY;
    } else if (cursorY >= viewOffsetY + rows - 1 && viewOffsetY + rows - 1 <= maxCursorY) {
        while (cursorY >= viewOffsetY + rows - 1 && viewOffsetY + rows - 1 <= maxCursorY) {
            viewOffsetY += 1;
        }
    } else if (cursorY < viewOffsetY) {
        while (cursorY < viewOffsetY) {
            viewOffsetY -= 1;
        }
    }

    int lineLength = textBuffer[cursorY].length();

    if (vertical) {
        // When moving vertically, try to maintain preferredCursorX
        if (lineLength > preferredCursorX) {
            cursorX = preferredCursorX;
        } else if (lineLength <= 1) {
            cursorX = 0;
        } else {
            cursorX = lineLength;
        }
    } else {
        // When moving horizontally, ensure cursorX is within the line
        if (cursorX < 0) {
            cursorX = 0;
            preferredCursorX = 0;
        } else if (cursorX >= lineLength) {
            cursorX = lineLength;
            if (mode == Mode::Command && cursorX >= 1)
                cursorX -= 1;
        }
    }
}

int Editor::getMaxCursorY() const {
    return textBuffer.size() - 1;
}

void Editor::setCommandParser(CommandParser& cp) {
    commandParserRef = &cp;
}

void Editor::run() {
    display->initialize();
    render();
    display->refresh();
    CommandParser commandParser(*this);
    setCommandParser(commandParser);

    ungetch('\n');
    while (isRunning) {
        int ch = getch();
        handleInput(ch, commandParser);
        render();
    }
}

void Editor::moveCursorToStart() {
    setCursorPosition(0, 0);
}

void Editor::moveCursorToEnd() {
    if (textBuffer.empty()) {
        setCursorPosition(0, 0);
        return;
    }

    int lastLine = textBuffer.size() - 1;
    if (lastLine < 0) {
        lastLine = 0;
    }
    setCursorPosition(lastLine, 0);
}

void Editor::moveCursorUpFrame() {
    int rows, cols;
    display->getWindowSize(rows, cols);
    int halfRows = rows / 2;
    viewOffsetY -= halfRows;
    if (viewOffsetY < 0) {
        viewOffsetY = 0;
    }
    cursorY -=  halfRows;
    cursorX = 0;
    ensureCursorInBounds(false);
}

void Editor::moveCursorDownFrame() {
    int maxCursorY = getMaxCursorY();
    int rows, cols;
    display->getWindowSize(rows, cols);
    int halfRows = rows / 2;
    viewOffsetY += halfRows;
    if (viewOffsetY + rows - 1 > maxCursorY) {
        viewOffsetY = maxCursorY + 1 - rows + 1;
    }
    cursorY +=  halfRows;
    cursorX = 0;
    ensureCursorInBounds(false);
}

void Editor::moveCursorForwardFrame() {
    int maxCursorY = getMaxCursorY();
    int rows, cols;
    display->getWindowSize(rows, cols);
    if (viewOffsetY + rows < maxCursorY) {
        viewOffsetY += rows;
    }
    cursorY +=  rows;
    cursorX = 0;
    ensureCursorInBounds(false);
}

void Editor::moveCursorBackwardFrame() {
    int rows, cols;
    display->getWindowSize(rows, cols);
    viewOffsetY -= rows;
    cursorY -=  rows;
    cursorX = 0;
    ensureCursorInBounds(false);
}

void Editor::insertLinesBelowCursor(const std::vector<std::string>& lines) {
    if (lines.empty()) {
        setStatusMessage("No content to insert.", true);
        return;
    }

    int insertionLine = cursorY + 1;

    textBuffer.insert(textBuffer.begin() + insertionLine, lines.begin(), lines.end());

    cursorY = insertionLine;
    cursorX = 0;
    preferredCursorX = 0;
}

void Editor::moveCursorBackWord() {
    if (cursorY < 0 || cursorY >= static_cast<int>(textBuffer.size())) {
        setStatusMessage("Invalid cursor position.", true);
        return;
    }
    std::string currentLine = textBuffer[cursorY];
    int y = cursorY;
    int x = cursorX;
    if (x == 0) {
        if (y == 0) {
            setStatusMessage("Already at the beginning of the file.", true);
            return;
        } else {
            y--;
            x = textBuffer[y].length();
            currentLine = textBuffer[y];
        }
    }
    if (x > 0) {
        x--;
    }
    while (y >= 0) {
        while (x >= 0 && std::isspace(currentLine[x])) {
            x--;
        }
        if (x < 0) {
            y--;
            if (y < 0) {
                y = 0;
                x = 0;
                break;
            }
            currentLine = textBuffer[y];
            x = textBuffer[y].length() - 1;
            continue;
        }
        while (x >= 0 && !std::isspace(currentLine[x])) {
            x--;
        }
        x++;
        cursorY = y;
        cursorX = x;

        setStatusMessage("Moved back by a word.", false);
        return;
    }

    cursorY = 0;
    cursorX = 0;
    setStatusMessage("Moved to the beginning of the file.", false);
}

void Editor::setClipboard(const std::vector<std::string>& content) {
    clipboardContent = content;
}

void Editor::clearClipboard() {
    clipboardContent.clear();
}

void Editor::appendClipboard(const std::string& content) {
    clipboardContent.push_back(content);
}

void Editor::enterVisualModeChar() {
    if (mode != Mode::Visual) {
        mode = Mode::Visual;
        visualModeHandler.startSelection(cursorY, cursorX, VisualType::Character);
        setStatusMessage("-- VISUAL --", false);
    } else {
        visualModeHandler.clearSelection();
        visualModeHandler.startSelection(cursorY, cursorX, VisualType::Character);
        setStatusMessage("-- VISUAL --", false);
    }
}

void Editor::enterVisualModeLine() {
    if (mode != Mode::Visual) {
        mode = Mode::Visual;
        visualModeHandler.startSelection(cursorY, cursorX, VisualType::Line);
        setStatusMessage("-- VISUAL LINE --", false);
    } else {
        visualModeHandler.clearSelection();
        visualModeHandler.startSelection(cursorY, cursorX, VisualType::Line);
        setStatusMessage("-- VISUAL LINE --", false);
    }
}

void Editor::enterVisualModeBlock() {
    if (mode != Mode::Visual) {
        mode = Mode::Visual;
        visualModeHandler.startSelection(cursorY, cursorX, VisualType::Block);
        setStatusMessage("-- VISUAL BLOCK --", false);
    } else {
        visualModeHandler.clearSelection();
        visualModeHandler.startSelection(cursorY, cursorX, VisualType::Block);
        setStatusMessage("-- VISUAL BLOCK --", false);
    }
}

void Editor::exitVisualMode() {
    if (mode == Mode::Visual) {
        mode = Mode::Command;
        visualModeHandler.clearSelection();
        setStatusMessage("", false);
    }
}

void Editor::updateVisualSelection() {
    if (mode == Mode::Visual) {
        visualModeHandler.updateSelection(cursorY, cursorX);
    }
}

void Editor::yankSelection() {
    if (mode == Mode::Visual && visualModeHandler.hasSelection()) {
        int startY, startX, endY, endX;
        visualModeHandler.getSelectionBounds(startY, startX, endY, endX);
        std::vector<std::string> selectedText;
        if (startY == endY) {
            std::string line = textBuffer[startY].substr(startX, endX - startX + 1);
            selectedText.push_back(line);
        } else {
            selectedText.push_back(textBuffer[startY].substr(startX));
            for (int l = startY + 1; l < endY; l++) {
                selectedText.push_back(textBuffer[l]);
            }
            selectedText.push_back(textBuffer[endY].substr(0, endX + 1));
        }
        setClipboard(selectedText);
        setStatusMessage("Yanked selection", false);
        exitVisualMode();
    }
}

void Editor::deleteSelection() {
    if (mode == Mode::Visual && visualModeHandler.hasSelection()) {
        int startY, startX, endY, endX;
        visualModeHandler.getSelectionBounds(startY, startX, endY, endX);
        std::vector<std::string> selectedText;
        if (startY == endY) {
            std::string line = textBuffer[startY].substr(startX, endX - startX + 1);
            selectedText.push_back(line);
        } else {
            selectedText.push_back(textBuffer[startY].substr(startX));
            for (int l = startY + 1; l < endY; l++) {
                selectedText.push_back(textBuffer[l]);
            }
            selectedText.push_back(textBuffer[endY].substr(0, endX + 1));
        }
        setClipboard(selectedText);

        if (startY == endY) {
            textBuffer[startY].erase(startX, endX - startX + 1);
        } else {
            textBuffer[startY].erase(startX);
            for (int l = startY + 1; l < endY; l++) {
                textBuffer.erase(textBuffer.begin() + (startY + 1));
            }
            std::string &lastLine = textBuffer[startY + 1];
            lastLine.erase(0, endX + 1);
            textBuffer[startY] += lastLine;
            textBuffer.erase(textBuffer.begin() + (startY + 1));
        }

        cursorY = startY;
        if (cursorX > (int)textBuffer[startY].size()) {
            cursorX = (int)textBuffer[startY].size();
        }

        setStatusMessage("Deleted selection", false);
        exitVisualMode();
    }
}

void Editor::recordLastChangeKeys(const std::string &keys) {
    lastChangeKeys = keys;
}

void Editor::repeatLastChange() {
    if (lastChangeKeys.empty()) {
        setStatusMessage("No last change to repeat", true);
        return;
    }
    replaying = true;
    for (char c : lastChangeKeys) {
        handleInput(c, *commandParserRef); 
    }
    replaying = false;
    ungetch('\n');
 
    setStatusMessage("Repeated last change", false);
}

