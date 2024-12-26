#include "FileManager.h"
#include <fstream>
#include <iostream>
#include <sys/stat.h> 

FileManager::FileManager() {
}

FileManager::~FileManager() {
}

bool FileManager::fileExists(const std::string& filename) const {
    struct stat buffer;
    return (stat(filename.c_str(), &buffer) == 0);
}

bool FileManager::loadFile(const std::string& filename, std::vector<std::string>& textBuffer) {
    std::ifstream infile(filename);
    if (!infile.is_open()) {
        std::cerr << "Error: Could not open file '" << filename << "' for reading.\n";
        return false;
    }

    textBuffer.clear(); 
    std::string line;
    while (std::getline(infile, line)) {
        textBuffer.emplace_back(line);
    }
    
    infile.close();
    return true;
}

bool FileManager::saveFile(const std::string& filename, const std::vector<std::string>& textBuffer) {
    std::ofstream outfile(filename);
    if (!outfile.is_open()) {
        std::cerr << "Error: Could not open file '" << filename << "' for writing.\n";
        return false;
    }

    for (const auto& line : textBuffer) {
        outfile << line << "\n";
    }

    outfile.close();
    return true;
}
