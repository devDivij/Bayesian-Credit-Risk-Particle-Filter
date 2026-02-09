#include "gen_pd.h"
#include "particle_grid.h"
#include <fstream>
#include <filesystem>
#include <sstream>
#include <cmath>

namespace
{
    constexpr double LAMBDA = 0.01; // Damping the effect of Crisis Correlation factor
    constexpr double INTERCEPT = -13.66;

    const std::vector<double> SystematicFactorSensitivities[] = {
        {-1.20, 0.15, 0.40, 1.50, 0.05}, // RETAIL
        {-0.90, 0.25, 0.60, 0.80, 0.30}, // CORPORATE
        {-0.50, 0.10, 0.30, 0.40, 0.10}, // INSTITUTIONS
        {-1.10, 0.30, 1.80, 1.20, 0.05}, // REALESTATE
        {-1.50, 0.80, 0.50, 0.20, 0.40}, // SOVEREIGN
    };
    const std::vector<double> IdiosyncraticFactorSensitivities[] = {
        {0.80, -0.50, 1.20}, // RETAIL
        {1.10, -0.30, 0.90}, // CORPORATE
        {0.60, -0.20, 0.50}, // INSTITUTIONS
        {1.40, -0.60, 1.10}, // REALESTATE
        {0.50, -0.10, 0.40}, // SOVEREIGN
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
        std::vector<double> crisisCorrelation = {std::max(0.00, -p.gdp), std::max(0.00, -p.inflation), std::max(0.00, -p.interest), std::max(0.00, -p.unemp), std::max(0.00, -p.oilPrice)};
        for (int j = 0; j < n_debtors; ++j)
        {
            Debtor d = debtors[j];
            std::vector<double> a = SystematicFactorSensitivities[d.exposure_class];
            std::vector<double> b = IdiosyncraticFactorSensitivities[d.exposure_class];
            for (size_t k = 0; k < a.size(); ++k)
                a[k] += LAMBDA * crisisCorrelation[k];

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