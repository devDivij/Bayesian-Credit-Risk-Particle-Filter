#pragma once
#include <iostream>
#include <vector>
#include <bitset>
#include <string>

struct Sobolparams
{
    int polynomial;
    int q;
    std::vector<int> minit;
};

struct Particle
{
    static constexpr int n_features = 5;
    union
    {
        struct
        {
            double gdp;       // GDP growth rate (%)
            double unemp;     // Unemployment rate (%)
            double inflation; // Inflation rate (%)
            double interest;  // Interest rate (%)
            double oilPrice;  // Oil price change (%)
        };
        double features[5];
    };
};

std::vector<Particle> genParticleGrid();

std::vector<Sobolparams> extract_Sobolparams(
    const std::string &filename,
    int dimensions);

std::vector<std::vector<double>> Sobolpts(
    int n0,
    int npts,
    int d,
    std::vector<Sobolparams> &S);

void initializeParticleWeights(const std::string &filename, std::vector<Particle> &particles);
void saveParticleGrid(const std::string &filename, const std::vector<Particle> &grid);