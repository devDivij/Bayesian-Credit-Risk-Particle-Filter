#include "update_weights.h"
#include <iostream>
#include <fstream>
#include <sstream>
#include <filesystem>
#include <cmath>
#include <algorithm>

namespace
{
    constexpr double EPS = 1e-10;

    std::vector<double> loadWeights(const std::string &f)
    {
        std::vector<double> weights;
        std::ifstream file(f);
        std::string line, last;

        while (std::getline(file, line))
            last = line;
        std::istringstream iss(last);
        double weight;
        while (iss >> weight)
        {
            weights.push_back(weight);
        }
        return weights;
    }

    std::vector<std::vector<double>> loadPDs(const std::string &f)
    {
        std::vector<std::vector<double>> PDs;
        std::ifstream file(f);
        std::string line;

        while (std::getline(file, line))
        {
            std::vector<double> conditional_pd;
            std::istringstream iss(line);
            double pd;

            while (iss >> pd)
            {
                conditional_pd.push_back(pd);
            }

            if (!conditional_pd.empty())
            {
                PDs.push_back(conditional_pd);
            }
        }

        return PDs;
    }

    std::vector<int> loadDefaults(const std::string &f)
    {
        std::vector<int> defaults;
        std::ifstream file(f);
        std::string line, last;

        while (std::getline(file, line))
            last = line;
        std::istringstream iss(last);
        int id;

        while (iss >> id)
        {
            defaults.push_back(id);
        }

        return defaults;
    }

    double calculateLogLikelihood(
        const std::vector<double> &conditional_pd,
        const std::vector<int> &defaults)
    {
        std::vector<char> is_default(conditional_pd.size(), 0);

        for (int idx : defaults)
        {
            if (idx > 0 && idx <= static_cast<int>(is_default.size()))
            {
                is_default[idx - 1] = 1;
            }
        }

        double log_likelihood = 0.0;

        for (size_t i = 0; i < conditional_pd.size(); ++i)
        {
            double pd = std::clamp(conditional_pd[i], EPS, 1.0 - EPS);

            log_likelihood += is_default[i]
                                  ? std::log(pd)
                                  : std::log1p(-pd);
        }

        return log_likelihood;
    }

    void saveWeights(
        const std::vector<double> &weights)
    {
        std::ofstream file("saved_state/weights.txt", std::ios::app);

        file << "\n";
        for (const auto &w : weights)
        {
            file << w << " ";
        }
    }
};

bool hasSavedState()
{
    return std::filesystem::exists("saved_state/weights.txt") && std::filesystem::file_size("saved_state/weights.txt") > 0;
}

void updateWeights()
{
    auto weights = loadWeights("saved_state/weights.txt");
    auto PDs = loadPDs("saved_state/debtor_pds.txt");
    auto defaults = loadDefaults("data/debtors/defaults.txt");
    std::cout << "Loaded " << weights.size() << " weights, " << PDs.size() << " PD sets, and "
              << defaults.size() << " defaults." << std::endl;

    std::vector<double> updated_log_weights(weights.size());
    std::vector<double> updated_weights;

    for (size_t i = 0; i < weights.size(); ++i)
    {
        double log_likelihood = calculateLogLikelihood(PDs[i], defaults);
        updated_log_weights[i] = std::log(weights[i]) + log_likelihood;
    }
    double maxLogW = *std::max_element(updated_log_weights.begin(), updated_log_weights.end());
    double sum = 0.0;
    for (auto &lw : updated_log_weights)
        sum += std::exp(lw - maxLogW);

    for (auto &w : updated_log_weights)
    {
        updated_weights.push_back(std::exp(w - maxLogW) / sum);
    }

    double sum_sq = 0.0;
    for (const auto &w : updated_weights)
    {
        sum_sq += w * w;
    }
    double ess = 1.0 / sum_sq;

    std::cout << "Effective Sample Size (ESS): " << ess << " / " << weights.size() << std::endl;
    std::cout << "ESS ratio: " << (ess / weights.size()) * 100 << "%" << std::endl;

    if (ess < weights.size() * 0.5)
    {
        std::cout << "WARNING: Low ESS - consider resampling particles!" << std::endl;
    }

    saveWeights(updated_weights);

    std::cout << "Weight update complete!" << std::endl;
}