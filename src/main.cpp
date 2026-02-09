#include <iostream>
#include "particle_grid.h"
#include "gen_pd.h"
#include "simulate_defaults.h"
#include "update_weights.h"
#include "run_python.h"

int main()
{
    if (hasSavedState())
    {
        std::cout << "Existing state detected. Updating weights with new defaults..." << std::endl;
        updateWeights();
        PythonAnalyzer::runAnalysis(
            "saved_state/particle_loss_ecdf.txt",
            "saved_state/weights.txt",
            "saved_state/ecdf_bins.txt");
    }
    else
    {

        std::cout << "No existing state found. Initializing from scratch..." << std::endl;
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
    }
    return 0;
}

// int main() {
//     try {
//         // Check if saved state exists
//         if (WeightUpdater::hasSavedState()) {
//             std::cout << "\n=== Existing State Detected ===" << std::endl;
//             std::cout << "Found existing particle filter state in 'saved_state' directory." << std::endl;
//             std::cout << "\nOptions:" << std::endl;
//             std::cout << "1. Update weights with new default data" << std::endl;
//             std::cout << "2. Reinitialize from scratch" << std::endl;
//             std::cout << "3. Exit" << std::endl;
//             std::cout << "\nEnter your choice (1/2/3): ";

//             int choice;
//             std::cin >> choice;

//             if (choice == 1) {
//                 // Check if new defaults file exists
//                 if (!fs::exists("data/new_defaults.txt")) {
//                     std::cout << "\nError: 'data/new_defaults.txt' not found!" << std::endl;
//                     std::cout << "Please create this file with new default observations (0s and 1s)." << std::endl;
//                     return 1;
//                 }

//                 // Update weights
//                 WeightUpdater::updateWeights();

//                 // Run risk analysis on updated weights
//                 std::cout << "\n=== Running Risk Analysis ===" << std::endl;
//                 SimplePythonAnalyzer analyzer;
//                 analyzer.runAnalysis(
//                     "saved_state/particle_loss_ecdf.txt",
//                     "saved_state/weights.txt",
//                     "saved_state/ecdf_bins.txt"
//                 );

//                 return 0;

//             } else if (choice == 2) {
//                 std::cout << "\nReinitializing from scratch..." << std::endl;
//                 // Continue to initialization below

//             } else {
//                 std::cout << "Exiting..." << std::endl;
//                 return 0;
//             }
//         }

//         // Original initialization code
//         std::cout << "\n=== Initializing Particle Filter ===" << std::endl;

//         // ... your existing initialization code ...
//         // (particle grid generation, PD generation, etc.)

//         std::cout << "\nInitialization complete!" << std::endl;

//     } catch (const std::exception& e) {
//         std::cerr << "Error: " << e.what() << std::endl;
//         return 1;
//     }

//     return 0;
// }