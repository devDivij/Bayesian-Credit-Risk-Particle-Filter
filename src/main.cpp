#include <iostream>
#include "particle_grid.h"
#include "gen_pd.h"
#include "simulate_defaults.h"
#include "run_python.h"

int main()
{
    std::vector<Particle> grid = genParticleGrid();
    std::vector<std::vector<double>> pd = genPDs();
    saveParticleGrid("saved_state/particle_grid.txt", grid);
    initializeParticleWeights("saved_state/weights.txt", grid);
    savePDs("saved_state/debtor_pds.txt", pd);
    std::vector<std::vector<double>> ecdf = genParticleLossECDFs();
    std::vector<double> bins = genTailConcentratedBins();
    saveECDFs("saved_state/particle_loss_ecdf.txt", ecdf);
    saveBins("saved_state/ecdf_bins.txt", bins);

    PythonAnalyzer::runAnalysis(
        "saved_state/particle_loss_ecdf.txt",
        "saved_state/weights.txt",
        "saved_state/ecdf_bins.txt");

    return 0;
}