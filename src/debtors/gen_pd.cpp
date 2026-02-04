#include "gen_pd.h"
#include "particle_grid.h"
#include <fstream>
#include <filesystem>
#include <sstream>
#include <cmath>

namespace
{
    constexpr double LAMBDA = 0.1; // Damping the effect of Crisis Correlation factor

    const std::vector<double> SystematicFactorSensitivities[] = {
        {0.1, 0.0, 0.9, 0.8, 0.0}, // Index 0: RETAIL
        {1.2, 0.8, 0.4, 0.3, 0.5}, // Index 1: CORPORATE
        {1.2, 0.8, 0.4, 0.3, 0.5}, // Index 2: INSTITUTIONS
        {1.2, 0.8, 0.4, 0.3, 0.5}, // Index 3: REALESTATE
        {1.2, 0.8, 0.4, 0.3, 0.5}, // Index 4: SOVEREIGN

    };
    const std::vector<double> IdiosyncraticFactorSensitivities[] = {
        {0.1, 0.0, 0.9, 0.8}, // Index 0: RETAIL
        {1.2, 0.8, 0.4, 0.3}, // Index 1: CORPORATE
        {1.2, 0.8, 0.4, 0.3}, // Index 2: INSTITUTIONS
        {1.2, 0.8, 0.4, 0.3}, // Index 3: REALESTATE
        {1.2, 0.8, 0.4, 0.3}, // Index 4: SOVEREIGN

    };

    std::vector<Debtor> getDebtorVariables(const std::string &filename)
    {
        std::ifstream file("data/debtors/debtor_data.txt");

        std::string line;
        std::getline(file, line);
        std::vector<Debtor> debtors;
        std::string temp_exposure;

        while (std::getline(file, line))
        {
            std::stringstream ss(line);
            Debtor d;
            ss >> d.id
            >> d.feature_1
            >> d.feature_2
            >> d.feature_3
            >> d.feature_4
            >> d.exposure_class;
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
    std::vector<std::vector<double>> PD (n_particles, std::vector<double> (n_debtors,0));

    for(int i=0; i<n_particles;++i){
        Particle p = particles[i];
        std::vector<double> crisisCorrelation = {std::max(0.0,-p.gdp),std::max(0.0,-p.inflation),std::max(0.0,-p.interest),std::max(0.0,-p.unemp),std::max(0.0,-p.oilPrice)};
        for (int j=0;j<n_debtors;++j){
            Debtor d=debtors[j];
            std::vector<double> a = SystematicFactorSensitivities[d.exposure_class];
            std::vector<double> b = IdiosyncraticFactorSensitivities[d.exposure_class];
            for (size_t k=0; k<a.size();++k) a[k]+= LAMBDA*crisisCorrelation[k];

            PD[i][j] = a[0]*p.gdp
                     + a[1]*p.inflation
                     + a[2]*p.interest
                     + a[3]*p.unemp
                     + a[4]*p.oilPrice
                     + b[0]*d.feature_1
                     + b[1]*d.feature_2
                     + b[2]*d.feature_3
                     + b[3]*d.feature_4;
            
                }
        
    }
    return PD;

}

void savePDs(const std::string& filename, const std::vector<std::vector<double>>& data) {
    std::ofstream outFile(filename);
    if (outFile.is_open()) {
        for (const auto& row : data) {
            for (size_t i = 0; i < row.size(); ++i) {
                outFile << row[i];
                if (i < row.size() - 1) outFile << " ";
            }
            outFile << "\n";
        }
        outFile.close();
    }
}