// Expected Loss: 8435.0682
// Unexpected Loss (Std Dev): 7227.5178
// Expected Shortfall (99%): 32841.6042
// VaR (99%): 29155.0
// VaR (95%): 21923.0
// Economic Capital: 29533.9318
// Tail Ratio (VaR_99.9 / VaR_99): 1.3023
#include "run_python.h"
#include <cstdlib>
#include <fstream>
#include <iostream>
#include <sstream>
#include <filesystem>

namespace fs = std::filesystem;

void PythonAnalyzer::runAnalysis(
    const std::string &ecdf_file,
    const std::string &weights_file,
    const std::string &bins_file)
{

// Use the Python script path defined by CMake
#ifndef PYTHON_SCRIPT_DIR
#define PYTHON_SCRIPT_DIR "."
#endif

    std::string scriptDir = PYTHON_SCRIPT_DIR;
    std::string scriptPath = scriptDir + "/risk_analysis.py";
    std::string outputPath = scriptDir + "/analysis_results.txt";

    if (!fs::exists(scriptPath))
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