#include "simulate_defaults.hpp"
#include "model_config.hpp"
#include "utils.hpp"
#include <random>
#include <iostream>
#include <fstream>
#include <sstream>
#include <algorithm>
#include <cmath>

using namespace Constants::SimulateDefaults;

namespace
{
    std::vector<Debtor> getDebtorVariables(const std::string &filename)
    {
        checkFileExists(filename);

        std::ifstream file(filename);

        std::string line;
        std::getline(file, line);
        std::vector<Debtor> debtors;

        while (std::getline(file, line))
        {
            std::stringstream ss(line);
            Debtor d;
            ss >> d.id;

            for (int i = 0; i < Debtor::n_features; ++i)
            {
                ss >> d.features[i];
            }

            ss >> d.EAD >> d.exposure_class;

            if (ss.fail())
            {
                std::cerr << "Warning: Malformed line in debtor data file: " << line << std::endl;
                continue;
            }
            debtors.push_back(d);
        }

        if (debtors.empty())
        {
            throw std::runtime_error("Error: Debtor data file is empty or invalid.");
        }
        return debtors;
    }
    std::vector<std::vector<double>> simulateLosses(const std::vector<std::vector<double>> &PDs)
    {
        std::random_device rd;
        std::mt19937 gen(rd());
        std::uniform_real_distribution<double> dist(0.0, 1.0);

        int n_particles = PDs.size();
        int n_debtors = PDs[0].size();

        std::vector<Debtor> debtors = getDebtorVariables("data/debtors/debtor_data.txt");

        if (debtors.size() != static_cast<size_t>(n_debtors))
        {
            throw std::runtime_error("Dimension mismatch: PD matrix dimensions do not match debtors in file (" + std::to_string(debtors.size()) + ").");
        }

        std::vector<std::vector<double>> losses(n_particles, std::vector<double>(N_ITER, 0.0));
        for (int i = 0; i < n_particles; ++i)
        {
            for (int j = 0; j < N_ITER; ++j)
            {
                double loss = 0.0;
                for (int k = 0; k < n_debtors; ++k)
                {
                    double rr = RecoveryRate[debtors[k].exposure_class];

                    if (rr < 0.0 || rr > 1.0)
                    {
                        throw std::out_of_range("RecoveryRate must be between 0 and 1 for class: " +
                                                std::to_string(debtors[k].exposure_class));
                    }

                    double u = dist(gen);
                    if (u < PDs[i][k])
                    {
                        loss += debtors[k].EAD * (1 - rr);
                    }
                }
                losses[i][j] = loss;
            }
        }
        return losses;
    }

}

std::vector<double> genTailConcentratedBins()
{
    if (TAIL_CONC <= 0)
        throw std::invalid_argument("TAIL_CONC must be positive.");
    if (MAX_LOSS <= MIN_LOSS)
        throw std::invalid_argument("MAX_LOSS must be greater than MIN_LOSS.");

    std::vector<double> bins(N_BINS);
    for (int i = 0; i < N_BINS; ++i)
    {
        double t = static_cast<double>(i) / (N_BINS - 1);
        double a = std::pow(t, 1.0 / TAIL_CONC);
        bins[i] = MIN_LOSS + a * (MAX_LOSS - MIN_LOSS);
    }
    return bins;
}

// ECDF: Empirical Cumulative Distrubution Function
std::vector<std::vector<double>> genParticleLossECDFs(const std::vector<std::vector<double>> &PDs)
{
    std::vector<std::vector<double>> losses = simulateLosses(PDs);
    std::vector<std::vector<double>> binned_ecdfs;
    std::vector<double> bins = genTailConcentratedBins();
    binned_ecdfs.reserve(losses.size());

    for (auto &l : losses)
    {
        std::sort(l.begin(), l.end());
        std::vector<double> ecdf;
        ecdf.reserve(bins.size());

        double n = static_cast<double>(N_ITER);
        if (n <= 0)
        {
            throw std::invalid_argument("N_ITER must be greater than 0 to calculate ECDF.");
        }
        for (const double &threshold : bins)
        {
            auto it = std::upper_bound(l.begin(), l.end(), threshold);
            double count = std::distance(l.begin(), it);
            ecdf.push_back(count / n);
        }

        binned_ecdfs.push_back(ecdf);
    }
    return binned_ecdfs;
}

void saveECDFs(const std::string &filename, const std::vector<std::vector<double>> &data)
{
    checkFileExists(filename);

    if (data.empty())
    {
        std::cerr << "Warning: No ECDF data to save to " << filename << std::endl;
        return;
    }

    std::ofstream outFile(filename);
    if (outFile.is_open())
    {
        for (const auto &row : data)
        {
            for (size_t i = 0; i < row.size(); ++i)
            {
                outFile << row[i];
                if (i < row.size() - 1)
                    outFile << " ";
            }
            outFile << "\n";
        }
        outFile.close();
    }
}

void saveBins(const std::string &filename, const std::vector<double> bins)
{
    checkFileExists(filename);

    if (bins.empty())
    {
        std::cerr << "Warning: No bins to save to " << filename << std::endl;
        return;
    }

    std::ofstream outFile(filename);
    if (outFile.is_open())
    {
        for (const auto &bin : bins)
        {
            outFile << std::ceil(bin);
            outFile << "\n";
        }
        outFile.close();
    }
}