#include <string>

class PythonAnalyzer
{
public:
    static void runAnalysis(
        const std::string &ecdf_file,
        const std::string &weights_file,
        const std::string &bins_file);
};