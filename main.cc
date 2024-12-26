// src/main.cpp
#include "Editor.h"
#include <iostream>

int main(int argc, char* argv[]) {
    std::string filename = "";
    if (argc == 2) {
        filename = std::string(argv[1]);
    }

    Editor editor(filename);
    editor.run();
    return 0;
}
