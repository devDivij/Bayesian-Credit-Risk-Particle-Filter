#include "update_weights.hpp"
#include "model_config.hpp"
#include "utils.hpp"
#include <iostream>
#include <fstream>
#include <sstream>
#include <filesystem>
#include <cmath>
#include <algorithm>
#include <vector>
#include <unordered_set>

using namespace Constants::Global;

namespace
{
    std::vector<double> loadWeights(const std::string &filename)
    {
        checkFileExists(filename);

        std::vector<double> weights;
        std::ifstream file(filename);
        std::string line, last;

        while (std::getline(file, line))
            last = line;

        std::istringstream iss(last);
        double weight;
        while (iss >> weight)
            weights.push_back(weight);

        return weights;
    }

    std::vector<std::vector<double>> loadPDs(const std::string &filename)
    {
        checkFileExists(filename);

        std::vector<std::vector<double>> PDs;
        std::ifstream file(filename);
        std::string line;

        while (std::getline(file, line))
        {
            std::vector<double> conditional_pds;
            std::istringstream iss(line);
            double pd;

            while (iss >> pd)
                conditional_pds.push_back(pd);

            if (!conditional_pds.empty())
                PDs.push_back(conditional_pds);
        }

        return PDs;
    }

    void loadDefaults(const std::string &filename, std::unordered_set<int> &all_defaults, std::unordered_set<int> &monthly_defaults)
    {
        checkFileExists(filename);

        std::ifstream file(filename);
        std::string line, last;

        while (std::getline(file, line))
        {
            if (!line.empty())
            {
                last = line;
                std::istringstream iss(line);
                int id;
                while (iss >> id)
                    all_defaults.insert(id);
            }
        }

        if (!last.empty())
        {
            std::istringstream iss(last);
            int id;
            while (iss >> id)
                monthly_defaults.insert(id);
        }
    }

    double calculateLogLikelihood(
        const std::vector<double> &conditional_pd,
        const std::unordered_set<int> &monthly_defaults,
        const std::unordered_set<int> &all_defaults)
    {

        double log_likelihood = 0.0;

        for (size_t i = 0; i < conditional_pd.size(); ++i)
        {
            int debtor_id = i + 1;

            if (all_defaults.count(debtor_id) && !monthly_defaults.count(debtor_id))
                continue;

            double pd = std::clamp(conditional_pd[i], EPS, 1.0 - EPS);
            log_likelihood += monthly_defaults.count(debtor_id) ? DEFAULT_TEMPERING * std::log(pd) : NON_DEFAULT_TEMPERING * std::log1p(-pd);
        }

        return log_likelihood;
    }

    void saveWeights(const std::vector<double> &weights)
    {
        checkFileExists("saved_state/weights.txt");

        std::ofstream file("saved_state/weights.txt", std::ios::app);
        file << "\n";
        for (size_t i = 0; i < weights.size(); ++i)
        {
            file << weights[i];
            if (i < weights.size() - 1)
                file << " ";
        }
    }
};

bool hasSavedState()
{
    return std::filesystem::exists("saved_state/weights.txt") &&
           std::filesystem::file_size("saved_state/weights.txt") > 0;
}

void updateWeights()
{
    auto weights = loadWeights("saved_state/weights.txt");
    auto PDs = loadPDs("saved_state/debtor_pds.txt");

    if (weights.empty() || PDs.empty())
    {
        std::cerr << "Error: Weights or PDs are empty. Check input files." << std::endl;
        return;
    }
    if (weights.size() != PDs.size())
    {
        std::cerr << "Error: Dimension mismatch. Weights: " << weights.size()
                  << " vs PD sets: " << PDs.size() << std::endl;
        return;
    }

    std::unordered_set<int> all_defaults;
    std::unordered_set<int> monthly_defaults;
    loadDefaults("data/debtors/defaults.txt", all_defaults, monthly_defaults);

    std::cout << "Loaded " << weights.size() << " weights, " << PDs.size()
              << " PD sets, "
              << monthly_defaults.size() << " this month's defaults." << std::endl;

    std::vector<double> updated_log_weights(weights.size());

    for (size_t i = 0; i < weights.size(); ++i)
    {
        double log_likelihood = calculateLogLikelihood(PDs[i], monthly_defaults, all_defaults);
        updated_log_weights[i] = std::log(weights[i] + EPS) + log_likelihood;
    }

    double maxLogW = *std::max_element(updated_log_weights.begin(), updated_log_weights.end());

    double sum = 0.0;
    std::vector<double> updated_weights(weights.size());

    for (size_t i = 0; i < weights.size(); ++i)
    {
        updated_weights[i] = std::exp(updated_log_weights[i] - maxLogW);
        sum += updated_weights[i];
    }

    for (size_t i = 0; i < weights.size(); ++i)
    {
        updated_weights[i] /= sum;
    }

    saveWeights(updated_weights);
    std::cout << "Weight update complete!" << std::endl;
}