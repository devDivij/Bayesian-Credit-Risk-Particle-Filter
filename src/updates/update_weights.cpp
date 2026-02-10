#include "update_weights.h"
#include <iostream>
#include <fstream>
#include <sstream>
#include <filesystem>
#include <cmath>
#include <algorithm>
#include <vector>
#include <unordered_set>

namespace
{
    constexpr double EPS = 1e-10;
    constexpr double TEMPERING = 0.01;

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
            weights.push_back(weight);

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
                conditional_pd.push_back(pd);

            if (!conditional_pd.empty())
                PDs.push_back(conditional_pd);
        }

        return PDs;
    }

    void loadDefaults(const std::string &f, std::unordered_set<int> &all_defaults, std::vector<int> &today_defaults)
    {
        std::ifstream file(f);
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
                today_defaults.push_back(id);
        }
    }

    double calculateLogLikelihood(
        const std::vector<double> &conditional_pd,
        const std::vector<int> &today_defaults,
        const std::unordered_set<int> &all_defaults)
    {
        std::unordered_set<int> today_set(today_defaults.begin(), today_defaults.end());
        std::vector<char> is_default_today(conditional_pd.size(), 0);

        for (int idx : today_defaults)
            is_default_today[idx - 1] = 1;

        double log_likelihood = 0.0;

        for (size_t i = 0; i < conditional_pd.size(); ++i)
        {
            int debtor_id = i + 1;

            if (all_defaults.count(debtor_id) && !today_set.count(debtor_id))
                continue;

            double pd = std::clamp(conditional_pd[i], EPS, 1.0 - EPS);
            log_likelihood += is_default_today[i] ? std::log(pd) : std::log1p(-pd);
        }

        return log_likelihood * TEMPERING;
    }

    void saveWeights(const std::vector<double> &weights)
    {
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

    std::unordered_set<int> all_defaults;
    std::vector<int> today_defaults;
    loadDefaults("data/debtors/defaults.txt", all_defaults, today_defaults);

    std::cout << "Loaded " << weights.size() << " weights, " << PDs.size()
              << " PD sets, " << all_defaults.size() << " cumulative defaults, and "
              << today_defaults.size() << " today's defaults." << std::endl;

    std::vector<double> updated_log_weights(weights.size());

    for (size_t i = 0; i < weights.size(); ++i)
    {
        double log_likelihood = calculateLogLikelihood(PDs[i], today_defaults, all_defaults);
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

    double sum_sq = 0.0;
    for (size_t i = 0; i < weights.size(); ++i)
    {
        updated_weights[i] /= sum;
        sum_sq += updated_weights[i] * updated_weights[i];
    }

    double ess = 1.0 / sum_sq;
    std::cout << "Effective Sample Size (ESS): " << ess << " / " << weights.size() << std::endl;
    std::cout << "ESS ratio: " << (ess / weights.size()) * 100 << "%" << std::endl;

    if (ess < weights.size() * 0.5)
        std::cout << "WARNING: Low ESS - consider resampling particles!" << std::endl;

    saveWeights(updated_weights);
    std::cout << "Weight update complete!" << std::endl;
}