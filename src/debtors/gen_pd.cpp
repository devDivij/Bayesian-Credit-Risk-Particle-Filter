#include "gen_pd.hpp"
#include "model_config.hpp"
#include "utils.hpp"
#include <fstream>
#include <filesystem>
#include <sstream>
#include <cmath>

using namespace Constants::DebtorPDs;

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

}

std::vector<std::vector<double>> genPDs(const std::vector<Particle> &particles)
{
    int n_particles = particles.size();
    std::vector<Debtor> debtors = getDebtorVariables("data/debtors/debtor_data.txt");
    int n_debtors = debtors.size();
    std::vector<std::vector<double>> PD(n_particles, std::vector<double>(n_debtors, 0));

    for (int i = 0; i < n_particles; ++i)
    {
        Particle p = particles[i];
        for (int j = 0; j < n_debtors; ++j)
        {
            Debtor d = debtors[j];
            const double *a = SystematicFactorSensitivities[d.exposure_class];
            const double *b = IdiosyncraticFactorSensitivities[d.exposure_class];

            double systematic_factor = 0.0;
            for (int k = 0; k < Particle::n_features; ++k)
            {
                systematic_factor += a[k] * p.features[k];
            }

            double idiosyncratic_factor = 0.0;
            for (int k = 0; k < Debtor::n_features; ++k)
            {
                idiosyncratic_factor += b[k] * d.features[k];
            }
            double z = INTERCEPT + systematic_factor + idiosyncratic_factor;

            if (std::isnan(z))
            {
                std::cerr << "Warning: NaN encountered in PD calculation for particle " << i + 1 << ", debtor " << d.id << std::endl;
            }
            else
            {
                PD[i][j] = (1.0 / (1.0 + std::exp(-z)));
            }
        }
    }
    return PD;
}

void savePDs(const std::string &filename, const std::vector<std::vector<double>> &data)
{
    checkFileExists(filename);

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