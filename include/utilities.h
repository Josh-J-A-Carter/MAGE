#ifndef UTILITIES_H
#define UTILITIES_H

#include <fstream>
#include <string>

std::string read_file(std::string filepath) {
    // Parse the file
    std::ifstream file { filepath };

    std::string output {};

    std::string line;
    while (std::getline(file, line)) output = output + line + "\n";

    return output;
}

#endif