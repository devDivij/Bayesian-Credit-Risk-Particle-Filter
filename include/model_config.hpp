#pragma once

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
        double features[n_features];
    };
};
struct Debtor
{

    static constexpr int n_features = 3;
    static constexpr int n_exposure_classes = 5;

    int id;
    union
    {
        struct
        {
            double feature_1;
            double feature_2;
            double feature_3;
        };
        double features[n_features];
    };
    double EAD;         // Exposure at Default
    int exposure_class; // 0: retail, 1: corporate, 2: institutions, 3: realEstate, 4: sovereign
};

namespace Constants
{
    namespace Global
    {
        constexpr double EPS = 1e-10; // Prevent values exploding
        constexpr int MSB = 32;       // Largest power of 2 required

        // Sensitivity of defaults affecting weights of each particle
        constexpr double DEFAULT_TEMPERING = 2.0;     // High for defaults
        constexpr double NON_DEFAULT_TEMPERING = 0.3; // Low for non-defaults
    }

    namespace ParticleGrid
    {
        // Macroeconomic Variables considered: GDP, Unemployment, Inflation, Interest Rates, Oil Prices
        constexpr int DIM = Particle::n_features; // Number of dimensions of particles
        constexpr int N_POINTS = 4e3;             // Not total particles, but number of Sobol points to generate

        // {GDP, Unemployment, Inflation, InterestRates, OilPrices}
        inline Particle mean_particle = {2.5, 5.5, 2.0, 3.5, 65.0};
        inline Particle sd = {3.5, 2.5, 3.5, 3.5, 35.0};

        // checkEconomicViability thresholds and coefficients are defined in check_viability.cpp itself
    }

    namespace DebtorPDs
    {
        // Coefficients for Logistic Regression model of generating PDs for each Debtor
        constexpr double INTERCEPT = -5.66;

        // {GDP, Unemployment, Inflation, InterestRates, OilPrices}
        inline const double SystematicFactorSensitivities[][Particle::n_features] = {
            {-0.40, 0.35, 0.10, 0.03, -0.005}, // RETAIL
            {-0.35, 0.40, 0.15, 0.04, -0.010}, // CORPORATE
            {-0.20, 0.20, 0.08, 0.03, -0.005}, // INSTITUTIONS
            {-0.50, 0.45, 0.28, 0.08, -0.005}, // REALESTATE
            {-0.55, 0.50, 0.12, 0.02, 0.020},  // SOVEREIGN
        };
        // {feature_1, feature_2, feature_3}
        inline const double IdiosyncraticFactorSensitivities[][Debtor::n_features] = {
            {0.08, -0.05, 0.12}, // RETAIL
            {0.11, -0.03, 0.09}, // CORPORATE
            {0.06, -0.02, 0.05}, // INSTITUTIONS
            {0.14, -0.06, 0.11}, // REALESTATE
            {0.05, -0.01, 0.04}, // SOVEREIGN
        };
    }
    namespace SimulateDefaults
    {

        constexpr int N_ITER = 500; // No. of iterations simulated for each particle

        // {retail, corporate, institutions, realEstate, sovereign}
        const double RecoveryRate[Debtor::n_exposure_classes] = {0.4, 0.4, 0.5, 0.7, 0.6};

        // Bin parameters (X-axis for Loss distrubution Functions)
        constexpr double TAIL_CONC = 1.5;
        constexpr int MIN_LOSS = 1;
        constexpr int MAX_LOSS = 2e6;
        constexpr int N_BINS = 400;

    }

}