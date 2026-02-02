#include "particle_grid.h"
#include <cmath>
#include <boost/math/distributions/normal.hpp>

// Constants and Assumptions
namespace
{
    // Macroeconomic Variables considered: GDP, Unemployment, Inflation, Interest Rates, Oil Prices
    constexpr int DIM = 5;          // Number of dimensions
    constexpr double EPS = 1e-10;   // Small epsilon to avoid quantile issues
    constexpr int N_POINTS = 62500; // Not total particles, but number of Sobol points to generate

    Particle mean_particle = {3.2, 5.0, 3.5, 5.0, 70.0}; // Mean values for each variable
    Particle sd = {2.5, 1.8, 3.0, 3.0, 30.0};            // Standard deviations for each variable

    // Check viability of a particle based on economic relationships
    bool checkEconomicViability(const Particle &p)
    {
        // Parameters
        double MEAN_UNEMP = 5.0;
        double OKUN_COEF = 0.4;
        double PHILLIPS_COEF = 0.5;
        double MEAN_INF = 2.0;
        double NEUTRAL_RATE = 2.0;
        double OIL_PRICE_COEF = 0.2;

        // Okun's Law: unemployment vs GDP
        double okun_residual = p.unemp - (MEAN_UNEMP - OKUN_COEF * p.gdp);
        if (abs(okun_residual) > 3.0)
            return false;

        // Phillips Curve: inflation vs unemployment
        double phillips_residual = p.inflation - (MEAN_INF - PHILLIPS_COEF * (p.unemp - MEAN_UNEMP));
        if (abs(phillips_residual) > 2.0)
            return false;

        // Taylor Rule: interest rate reacts to inflation and GDP
        double taylor_rate = NEUTRAL_RATE + p.inflation + 0.5 * (p.inflation - MEAN_INF) + 0.5 * p.gdp;
        if (abs(p.interest - taylor_rate) > 3.0)
            return false;

        // Oil up -> inflation up
        double oil_infl_residual = p.inflation - OIL_PRICE_COEF * p.oilPrice;
        if (abs(oil_infl_residual) > 4.0)
            return false;

        // Recession coherence: If GDP strongly negative, unemployment must rise
        if (p.gdp < -2.0 && p.unemp < MEAN_UNEMP)
            return false;

        // If inflation is high, interest rates cannot be very low
        if (p.inflation > 4.0 && p.interest < 1.0)
            return false;

        // Oil shock: oil up + GDP up + inflation down is implausible
        if (p.oilPrice > 10.0 && p.gdp > 2.0 && p.inflation < 0.0)
            return false;

        // Feasible bounds
        if (p.gdp < -10.0 || p.gdp > 10.0)
            return false;
        if (p.unemp < 1.0 || p.unemp > 25.0)
            return false;
        if (p.inflation < -5.0 || p.inflation > 20.0)
            return false;
        if (p.interest < -2.0 || p.interest > 15.0)
            return false;
        if (p.oilPrice < 5.0 || p.oilPrice > 150.0)
            return false;

        return true;
    }

    double clamp(double val)
    {
        return std::max(EPS, std::min(1 - EPS, val));
    }
}

std::vector<Particle> getParticleGrid()
{
    // Generate Sobol points
    std::vector<Sobolparams> S = extract_Sobolparams("data/particle_grid/new-joe-kuo-6.21201", DIM);
    std::vector<std::vector<double>> P = Sobolpts(1, N_POINTS, DIM, S);

    boost::math::normal_distribution<double> dist(0.0, 1.0);

    // Transform Sobol points to particles
    std::vector<Particle> particles;
    for (const auto &point : P)
    {
        Particle p;
        p.gdp = mean_particle.gdp + sd.gdp * quantile(dist, clamp(point[0]));
        p.unemp = mean_particle.unemp + sd.unemp * quantile(dist, clamp(point[1]));
        p.inflation = mean_particle.inflation + sd.inflation * quantile(dist, clamp(point[2]));
        p.interest = mean_particle.interest + sd.interest * quantile(dist, clamp(point[3]));
        p.oilPrice = mean_particle.oilPrice + sd.oilPrice * quantile(dist, clamp(point[4]));
        if (checkEconomicViability(p))
        {
            particles.push_back(p);
        }
    }
    return particles;
}
