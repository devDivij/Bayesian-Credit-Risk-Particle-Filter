#include <iostream>
#include "particle_grid.hpp"
#include "gen_pd.hpp"
#include "simulate_defaults.hpp"
#include "update_weights.hpp"
#include "run_python.hpp"

int main()
{
    if (hasSavedState())
    {
        std::cout << "Existing state detected." << std::endl;

        bool x = 1;
        // std::cout << "Do you want to update weights with new defaults? (1 for yes, 0 for no): ";
        // std::cin >> x;
        if (x)
        {
            updateWeights();
        }
        PythonAnalyzer::runAnalysis(
            "saved_state/particle_loss_ecdf.txt",
            "saved_state/weights.txt",
            "saved_state/ecdf_bins.txt");
    }
    else
    {

        std::cout << "No existing state found. Initializing from scratch..." << std::endl;
        std::vector<Particle> particles = genParticleGrid();
        std::vector<std::vector<double>> PDs = genPDs(particles);
        saveParticleGrid("saved_state/particle_grid.txt", particles);
        initializeParticleWeights("saved_state/weights.txt", particles);
        savePDs("saved_state/debtor_pds.txt", PDs);
        std::vector<std::vector<double>> ecdf = genParticleLossECDFs(PDs);
        std::vector<double> bins = genTailConcentratedBins();
        saveECDFs("saved_state/particle_loss_ecdf.txt", ecdf);
        saveBins("saved_state/ecdf_bins.txt", bins);

        PythonAnalyzer::runAnalysis(
            "saved_state/particle_loss_ecdf.txt",
            "saved_state/weights.txt",
            "saved_state/ecdf_bins.txt");
    }
    return 0;
}
