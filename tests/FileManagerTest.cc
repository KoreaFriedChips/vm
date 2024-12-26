#include "FileManager.h"
#include <iostream>

int main() {
    FileManager fileManager;
    std::vector<std::string> textBuffer;

    // Test Loading a File
    std::string filename = "test.txt";
    std::cout << "Loading file: " << filename << "\n";
    if (fileManager.loadFile(filename, textBuffer)) {
        std::cout << "File loaded successfully. Contents:\n";
        for (const auto& line : textBuffer) {
            std::cout << line << "\n";
        }
    } else {
        std::cout << "Failed to load file.\n";
    }

    // Modify the text buffer
    textBuffer.emplace_back("This line was added during the test.");

    // Test Saving the File
    std::string saveFilename = "test_saved.txt";
    std::cout << "\nSaving modified content to: " << saveFilename << "\n";
    if (fileManager.saveFile(saveFilename, textBuffer)) {
        std::cout << "File saved successfully.\n";
    } else {
        std::cout << "Failed to save file.\n";
    }

    return 0;
}
