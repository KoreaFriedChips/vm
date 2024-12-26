#ifndef EDITOR_H
#define EDITOR_H

#include "Display.h"
#include "FileManager.h"
#include "StatusBar.h"
#include <memory>
#include <vector>
#include <unordered_map>
#include <string>
#include <stack>
#include "ICommand.h"
#include "TokenType.h"
#include "VisualModeHandler.h"

enum class Mode {
    Command,
    Insert,
    Replace,
    Visual
};

class CommandParser;


class Editor {
public:
    Editor(const std::string& filename = "");
    ~Editor();

    void run();

    int getCursorY() const;
    int getCursorX() const;
    void setCursorPosition(int y, int x);
    int getMaxCursorY() const;

    void insertCharacter(int ch);
    void deleteCharacter(bool type);
    char getCharacterAtCursor() const;

    void executeCommand(std::shared_ptr<ICommand> command);

    Mode getMode() const;
    void switchMode(Mode newMode);

    void setFilename(const std::string& fname);
    std::string getFilename() const;

    bool loadFile(const std::string& filename);
    bool saveFile(const std::string& filename);

    void setIsRunning(bool running);

    void setStatusMessage(const std::string& message, bool isError = false);

    bool getIsModified() const;

    void moveCursorToStart();
    void moveCursorToEnd();

    void insertLinesBelowCursor(const std::vector<std::string>& lines);

    void moveCursor(int deltaY, int deltaX);

    void moveCursorBackWord();

    std::vector<std::string> textBuffer;

    std::stack<std::pair<std::vector<std::string>, std::pair<int, int>>> fileHistory;

    int cursorY;
    int cursorX;
    int preferredCursorX;

    void render();

    void setClipboard(const std::vector<std::string>& content);
    std::vector<std::string> clipboardContent;
    void clearClipboard();
    void appendClipboard(const std::string& content);

    void moveCursorUpFrame();
    void moveCursorDownFrame();
    void moveCursorForwardFrame();
    void moveCursorBackwardFrame();

    // Store input
    std::vector<char> input;
    std::vector<char> backup; // for repeat last command
    void handleInsertToCommand();
    int lastMult;
    int backupMult;
    std::string commandSeq;
    std::string savedCommandSeq;
    std::string lastCommand; // for commands that don't require this side effect
    // Repeat last command that changes the file
    std::string lastChangeKeys; // store last change keys
    bool replaying; // are we currently replaying keystrokes for '.'

    void recordLastChangeKeys(const std::string &keys);
    void repeatLastChange();
    
    void enterVisualModeChar();   
    void enterVisualModeLine();   
    void enterVisualModeBlock();  
    void exitVisualMode();
    void updateVisualSelection(); 
    void yankSelection();
    void deleteSelection();

    void setCommandParser(CommandParser& cp);

private:
    VisualModeHandler visualModeHandler;
    std::shared_ptr<Display> display;
    std::shared_ptr<FileManager> fileManager;
    std::unique_ptr<StatusBar> statusBar;
    Mode mode;
    bool isRunning;
    bool enhanced;

    std::string filename;
    std::vector<std::string> unmodifiedFile;

    int viewOffsetY; 

    void handleInput(int ch, CommandParser& commandParser);
    void ensureCursorInBounds(bool vertical);

    CommandParser* commandParserRef;
};

#endif
