#include "simulate_defaults.h"
#include "gen_pd.h"
#include <random>
#include <fstream>
#include <sstream>
#include <algorithm>
#include <cmath>

namespace
{
    constexpr int N_ITER = 5000;

    // Bin parameters
    constexpr double TAIL_CONC = 1.5;
    constexpr int MIN_LOSS = 1;
    constexpr int MAX_LOSS = 150000;
    constexpr int N_BINS = 400;

    const double RecoveryRate[] = {0.4, 0.4, 0.5, 0.7, 0.6};

    std::vector<std::pair<int, double>> getExposureAtDefault(const std::string &filename)
    {
        std::ifstream file(filename);

        std::string line;
        std::getline(file, line);
        std::vector<std::pair<int, double>> EAD_vec;

        while (std::getline(file, line))
        {
            std::stringstream ss(line);
            Debtor d;
            ss >> d.id >> d.feature_1 >> d.feature_2 >> d.feature_3 >> d.EAD >> d.exposure_class;
            EAD_vec.push_back({d.exposure_class, d.EAD});
        }
        return EAD_vec;
    }
    std::vector<std::vector<double>> simulateLosses()
    {
        std::random_device rd;
        std::mt19937 gen(rd());
        std::uniform_real_distribution<double> dist(0.0, 1.0);

        std::vector<std::vector<double>> PD = genPDs();
        int n_particles = PD.size();
        int n_debtors = PD[0].size();

        std::vector<std::pair<int, double>> EAD = getExposureAtDefault("data/debtors/debtor_data.txt");

        std::vector<std::vector<double>> losses(n_particles, std::vector<double>(N_ITER, 0.0));
        for (int i = 0; i < n_particles; ++i)
        {
            for (int j = 0; j < N_ITER; ++j)
            {
                double loss = 0.0;
                for (int k = 0; k < n_debtors; ++k)
                {
                    double u = dist(gen);
                    if (u < PD[i][k])
                    {
                        loss += EAD[k].second * (1 - RecoveryRate[EAD[k].first]);
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
std::vector<std::vector<double>> genParticleLossECDFs()
{
    std::vector<std::vector<double>> losses = simulateLosses();
    std::vector<std::vector<double>> binned_ecdfs;
    std::vector<double> bins = genTailConcentratedBins();
    binned_ecdfs.reserve(losses.size());

    for (auto &l : losses)
    {
        std::sort(l.begin(), l.end());
        std::vector<double> ecdf;
        ecdf.reserve(bins.size());

        double n = static_cast<double>(N_ITER);

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