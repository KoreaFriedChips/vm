#ifndef FILEMANAGER_H
#define FILEMANAGER_H

#include <string>
#include <vector>

class FileManager {
public:
    FileManager();
    ~FileManager();

    bool loadFile(const std::string& filename, std::vector<std::string>& textBuffer);
    bool saveFile(const std::string& filename, const std::vector<std::string>& textBuffer);
    bool fileExists(const std::string& filename) const;
};

#endif
