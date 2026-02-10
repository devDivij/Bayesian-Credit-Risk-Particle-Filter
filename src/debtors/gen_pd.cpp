#include "gen_pd.h"
#include "particle_grid.h"
#include <fstream>
#include <filesystem>
#include <sstream>
#include <cmath>

namespace
{
    constexpr double INTERCEPT = -5.66;

    const std::vector<double> SystematicFactorSensitivities[] = {
        {-0.12, 0.015, 0.04, 0.15, 0.005}, // RETAIL
        {-0.09, 0.025, 0.06, 0.08, 0.030}, // CORPORATE
        {-0.05, 0.010, 0.03, 0.04, 0.010}, // INSTITUTIONS
        {-0.11, 0.030, 0.18, 0.12, 0.005}, // REALESTATE
        {-0.15, 0.080, 0.05, 0.02, 0.040}, // SOVEREIGN
    };

    const std::vector<double> IdiosyncraticFactorSensitivities[] = {
        {0.08, -0.05, 0.12}, // RETAIL
        {0.11, -0.03, 0.09}, // CORPORATE
        {0.06, -0.02, 0.05}, // INSTITUTIONS
        {0.14, -0.06, 0.11}, // REALESTATE
        {0.05, -0.01, 0.04}, // SOVEREIGN
    };

    std::vector<Debtor> getDebtorVariables(const std::string &filename)
    {
        std::ifstream file(filename);

        std::string line;
        std::getline(file, line);
        std::vector<Debtor> debtors;

        while (std::getline(file, line))
        {
            std::stringstream ss(line);
            Debtor d;
            ss >> d.id >> d.feature_1 >> d.feature_2 >> d.feature_3 >> d.EAD >> d.exposure_class;
            debtors.push_back(d);
        }
        return debtors;
    }

}

std::vector<std::vector<double>> genPDs()
{
    std::vector<Particle> particles = genParticleGrid();
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
            std::vector<double> a = SystematicFactorSensitivities[d.exposure_class];
            std::vector<double> b = IdiosyncraticFactorSensitivities[d.exposure_class];

            double z = INTERCEPT + a[0] * p.gdp + a[1] * p.inflation + a[2] * p.interest + a[3] * p.unemp + a[4] * p.oilPrice + b[0] * d.feature_1 + b[1] * d.feature_2 + b[2] * d.feature_3;
            PD[i][j] = (1 / (1 + std::exp(-z)));
        }
    }
    return PD;
}

void savePDs(const std::string &filename, const std::vector<std::vector<double>> &data)
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