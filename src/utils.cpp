#include "utils.hpp"
#include <iostream>
#include <fstream>

void checkFileExists(const std::string &filename)
{
    std::ifstream file(filename);
    if (!file.is_open())
    {
        throw std::runtime_error("Error: Cannot open Sobol parameters file: " + filename);
    }
}