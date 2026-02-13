#include "particle_grid.hpp"
#include "model_config.hpp"
#include "utils.hpp"
#include <cmath>
#include <fstream>
#include <algorithm>
#include <boost/math/distributions/normal.hpp>

using namespace Constants::Global;
using namespace Constants::ParticleGrid;

std::vector<Particle> genParticleGrid()
{
    // Generate Sobol points
    std::vector<Sobolparams> S = extractSobolparams("data/particle_grid/new-joe-kuo-6.21201", DIM);
    if (S.empty())
    {
        throw std::runtime_error("Failed to load Sobol parameters");
    }

    std::vector<std::vector<double>> P = Sobolpts(1, N_POINTS, DIM, S);

    boost::math::normal_distribution<double> dist(0.0, 1.0);

    // Transform Sobol points to particles
    std::vector<Particle> particles;
    particles.reserve(N_POINTS);

    for (const auto &point : P)
    {
        Particle p;
        for (int i = 0; i < Particle::n_features; ++i)
        {
            p.features[i] = mean_particle.features[i] + sd.features[i] * quantile(dist, std::clamp(point[i], EPS, 1.0 - EPS));
        }
        if (checkEconomicViability(p))
        {
            particles.push_back(p);
        }
    }
    if (particles.empty())
    {
        throw std::runtime_error("No particles survived viability check.");
    }
    return particles;
}

void initializeParticleWeights(const std::string &filename, std::vector<Particle> &particles)
{
    checkFileExists(filename);

    int n_particles = particles.size();
    double weight = 1.0 / n_particles;
    std::ofstream outFile(filename);
    if (outFile.is_open())
    {
        for (int i = 0; i < n_particles; ++i)
        {
            outFile << weight << " ";
        }
        outFile.close();
    }
}

void saveParticleGrid(const std::string &filename, const std::vector<Particle> &grid)
{
    checkFileExists(filename);

    std::ofstream outFile(filename);
    if (outFile.is_open())
    {
        for (const auto &particle : grid)
        {
            for (size_t i = 0; i < Particle::n_features; ++i)
            {
                outFile << particle.features[i];
                if (i < Particle::n_features - 1)
                    outFile << " ";
            }
            outFile << "\n";
        }
        outFile.close();
    }
}