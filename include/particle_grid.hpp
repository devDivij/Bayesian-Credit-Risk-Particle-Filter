#pragma once

#include "model_config.hpp"
#include <iostream>
#include <vector>
#include <bitset>
#include <string>

struct Sobolparams
{
    int polynomial;
    int q;
    std::vector<int> m_init;
};

bool checkEconomicViability(const Particle &p);

std::vector<Particle> genParticleGrid();

std::vector<Sobolparams> extractSobolparams(
    const std::string &filename,
    int dimensions);

std::vector<std::vector<double>> Sobolpts(
    int n0,
    int npts,
    int d,
    std::vector<Sobolparams> &S);

void initializeParticleWeights(const std::string &filename, std::vector<Particle> &particles);
void saveParticleGrid(const std::string &filename, const std::vector<Particle> &grid);