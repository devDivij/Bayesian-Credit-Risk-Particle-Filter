#include "run_python.hpp"
#include <cstdlib>
#include <fstream>
#include <iostream>
#include <sstream>
#include <filesystem>

#ifndef PYTHON_SCRIPT_DIR
#define PYTHON_SCRIPT_DIR "."
#endif

void PythonAnalyzer::runAnalysis(
    const std::string &ecdf_file,
    const std::string &weights_file,
    const std::string &bins_file)
{

    if (!std::filesystem::exists(ecdf_file) ||
        !std::filesystem::exists(weights_file) ||
        !std::filesystem::exists(bins_file))
    {
        throw std::runtime_error("Required data files for Python analysis are missing.");
    }
    if (std::system("python3 --version > /dev/null 2>&1") != 0)
    {
        throw std::runtime_error("python3 command not found. Please ensure Python is installed and in your PATH.");
    }

    std::string scriptDir = PYTHON_SCRIPT_DIR;
    std::string scriptPath = scriptDir + "/risk_analysis.py";
    std::string outputPath = scriptDir + "/analysis_results.txt";

    if (!std::filesystem::exists(scriptPath))
    {
        throw std::runtime_error("Python script not found at: " + scriptPath);
    }

    std::string command = "cd \"" + scriptDir + "\" && python3 risk_analysis.py > analysis_results.txt";

    int result = std::system(command.c_str());
    if (result != 0)
    {
        throw std::runtime_error("Python script execution failed");
    }

    // Read results
    std::ifstream file(outputPath);
    if (!file.is_open())
    {
        throw std::runtime_error("Could not open " + outputPath);
    }

    std::string line;
    while (std::getline(file, line))
    {
        std::cout << line << std::endl;
    }
}