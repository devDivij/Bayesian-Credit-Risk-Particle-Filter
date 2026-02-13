#include "particle_grid.hpp"
#include "model_config.hpp"

using namespace Constants::ParticleGrid;

bool checkEconomicViability(const Particle &p)
{
    // Natural rate of unemployment
    constexpr double MEAN_UNEMP = 5.5;

    // Okun's coefficient: how much unemployment changes with GDP
    constexpr double OKUN_COEF = 0.45;

    // Phillips curve coefficient: relationship between unemployment and inflation
    constexpr double PHILLIPS_COEF = 0.4;

    // Mean inflation rate
    constexpr double MEAN_INF = 2.0;

    // Neutral interest rate (rate that neither stimulates nor restricts growth)
    constexpr double NEUTRAL_RATE = 2.0;

    // How much oil price changes affect inflation (pass-through coefficient)
    constexpr double OIL_PRICE_COEF = 0.015;

    // OKUN'S LAW: GDP Growth vs Unemployment
    // When GDP grows, unemployment falls, and vice versa
    double expected_unemp = MEAN_UNEMP - OKUN_COEF * p.gdp;
    double okun_residual = p.unemp - expected_unemp;

    // Allow larger deviations during extreme economic conditions
    double okun_tolerance = 3.5 + 0.1 * std::abs(p.gdp);
    if (std::abs(okun_residual) > okun_tolerance)
        return false;

    // PHILLIPS CURVE: Unemployment vs Inflation
    // Lower unemployment typically leads to higher inflation
    double expected_inflation = MEAN_INF - PHILLIPS_COEF * (p.unemp - MEAN_UNEMP);
    double phillips_residual = p.inflation - expected_inflation;

    if (std::abs(phillips_residual) > 3.5)
        return false;

    // TAYLOR RULE: Interest Rate Policy
    double taylor_rate = NEUTRAL_RATE + p.inflation +
                         0.5 * (p.inflation - MEAN_INF) +
                         0.5 * p.gdp;

    double taylor_tolerance = 4.0;

    // During crisis (very negative GDP), allow central banks to cut rates aggressively even below what Taylor rule suggests
    if (p.gdp < -3.0)
    {
        taylor_tolerance = 6.0;
    }
    if (p.gdp > 4.0)
    {
        taylor_tolerance = 5.0;
    }

    if (std::abs(p.interest - taylor_rate) > taylor_tolerance)
        return false;

    // OIL PRICE EFFECTS ON INFLATION
    // Oil is a major input cost, so oil prices affect overall inflation
    double oil_deviation = p.oilPrice - mean_particle.oilPrice;
    double expected_oil_inflation_impact = OIL_PRICE_COEF * oil_deviation;
    double oil_infl_residual = p.inflation - MEAN_INF - expected_oil_inflation_impact;

    if (std::abs(oil_infl_residual) > 5.0)
        return false;

    // RECESSION COHERENCE CHECKS
    // During deep recessions, certain combinations are impossible

    // During severe recession (GDP < -3%), unemployment must be elevated
    if (p.gdp < -3.0 && p.unemp < MEAN_UNEMP - 1.0)
        return false;

    // During deep recession, we typically see low inflation or deflation
    // High inflation during recession (stagflation) is rare but possible during supply shocks
    if (p.gdp < -4.0 && p.inflation > 5.0 && p.oilPrice < 90.0)
        return false; // Stagflation only makes sense with oil shock

    // EXPANSION COHERENCE CHECKS
    // During strong expansions, certain patterns must hold

    // During strong expansion (GDP > 4%), unemployment should be low
    if (p.gdp > 4.0 && p.unemp > MEAN_UNEMP + 1.5)
        return false;

    // Very strong growth typically brings inflationary pressure
    if (p.gdp > 5.0 && p.inflation < -1.0)
        return false;

    // MONETARY POLICY COHERENCE
    // Interest rates must respond sensibly to inflation

    // If inflation is very high (>6%), central banks must raise rates
    if (p.inflation > 6.0 && p.interest < p.inflation - 2.0)
        return false;

    // If inflation is very low or negative, rates should be low
    if (p.inflation < 0.0 && p.interest > 5.0)
        return false;

    // OIL SHOCK COHERENCE
    // Oil shocks have predictable effects on the economy

    // Major oil price spike typically causes inflation and slows growth
    if (p.oilPrice > 100.0 && p.gdp > 3.0 && p.inflation < 1.0)
        return false;

    // Very low oil prices should boost growth and reduce inflation
    if (p.oilPrice < 40.0 && p.gdp < -2.0 && p.inflation > 4.0)
        return false;

    // DEFLATION TRAP CHECK
    // Deflation with high unemployment and low rates indicates severe crisis

    // Allow deflation scenarios but ensure they're consistent with crisis conditions
    if (p.inflation < -2.0)
    {
        if (p.gdp > 1.0 || p.unemp < MEAN_UNEMP)
            return false;
    }

    // HARD BOUNDS ON VARIABLES
    // These represent the extreme limits of what's economically possible

    if (p.gdp < -10.0 || p.gdp > 10.0)
        return false;

    if (p.unemp < 1.0 || p.unemp > 25.0)
        return false;

    if (p.inflation < -4.0 || p.inflation > 18.0)
        return false;

    if (p.interest < -1.0 || p.interest > 15.0)
        return false;

    if (p.oilPrice < 15.0 || p.oilPrice > 160.0)
        return false;

    // ===================================================================
    // ADDITIONAL COHERENCE CHECKS FOR EXTREME STATES
    // ===================================================================

    // Crisis state: Multiple indicators must be in distress simultaneously
    bool severe_gdp_decline = p.gdp < -4.0;
    bool high_unemployment = p.unemp > 8.0;
    bool deflation = p.inflation < 0.0;

    // If we have severe GDP decline, we must have at least one other crisis indicator
    if (severe_gdp_decline && !high_unemployment && !deflation && p.interest > 3.0)
        return false;

    // Expansion state: Multiple positive indicators should align
    bool strong_growth = p.gdp > 4.0;
    bool low_unemployment = p.unemp < 4.0;
    bool moderate_inflation = p.inflation > 1.5 && p.inflation < 4.0;

    // Strong expansion should have low unemployment
    if (strong_growth && p.unemp > 6.5)
        return false;

    return true;
}
