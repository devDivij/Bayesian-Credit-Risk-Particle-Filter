#include <iostream>
#include "particle_grid.h"
#include "gen_pd.h"
#include "simulate_defaults.h"

int main()
{
    std::vector<Particle> grid = genParticleGrid();
    std::vector<std::vector<double>> pd = genPDs();
    std::cout << "Total PDs generated: " << (pd.empty() ? 0 : pd.size() * pd[0].size()) << std::endl;
    saveParticleGrid("saved_state/particle_grid.txt", grid);
    savePDs("saved_state/debtor_pds.txt", pd);
    std::vector<std::vector<double>> ecdf = genParticleLossECDFs();
    std::vector<double> bins = genBins();
    saveECDFs("saved_state/particle_loss_ecdf.txt", ecdf);
    saveBins("saved_state/ecdf_bins.txt", bins);
    return 0;
}