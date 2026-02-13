#include "particle_grid.hpp"
#include "model_config.hpp"
#include "utils.hpp"
#include <iostream>
#include <cmath>
#include <fstream>
#include <bitset>
#include <string>
#include <sstream>

using namespace Constants::Global;

namespace
{
    // Helper functions
    std::vector<std::vector<bool>> Sobolmat(std::bitset<MSB> c, int q, int r, std::vector<int> m)
    {
        std::vector<bool> a(q - 1, 0);
        std::vector<std::vector<bool>> V(r, std::vector<bool>(r, 0));

        m.resize(r, 0);

        if (q)
        {
            for (int i = 0; i < q - 1; ++i)
            {
                a[i] = c[q - 2 - i];
            }
            for (int j = q - 1; j < r; ++j)
            {
                for (int i = 0; i < q - 1; ++i)
                {
                    m[j] ^= (1 << (i + 1)) * a[i] * m[j - i - 1];
                }
                m[j] ^= ((1 << (q)) * m[j - q]) ^ m[j - q];
            }

            for (int j = 0; j < r; ++j)
            {
                int val = m[j];
                for (int i = 0; i <= j; ++i)
                {
                    V[i][j] = ((val >> (j - i)) & 1);
                }
            }
        }
        else
        {
            for (int i = 0; i < r; ++i)
            {
                for (int j = 0; j < r; ++j)
                {
                    if (i == j)
                    {
                        V[i][j] = 1;
                    }
                }
            }
        }

        return V;
    }

    int rightmostbit(int n)
    {
        int pos = 0;
        while (n & 1)
        {
            n >>= 1;
            pos++;
        }
        return pos;
    }

    // Validation functions
    void isValidSobolParams(const Sobolparams &s, const int &dimension)
    {
        if (s.m_init.empty())
        {
            throw std::invalid_argument("Error: m_init vector is empty for dimension " + std::to_string(dimension));
        }
        if (s.q < 0 || s.q > MSB)
        {
            throw std::invalid_argument("Error: Invalid q value " + std::to_string(s.q) + " for dimension " + std::to_string(dimension));
        }
        if (s.polynomial < 0)
        {
            throw std::invalid_argument("Error: Invalid polynomial " + std::to_string(s.polynomial) + " for dimension " + std::to_string(dimension));
        }
    }

    void validateInput(const int &n0, const int &npts, const int &d, const std::vector<Sobolparams> &S)
    {
        if (n0 <= 0)
        {
            throw std::invalid_argument("Error: n0 must be positive, got " + std::to_string(n0));
        }
        if (npts <= 0)
        {
            throw std::invalid_argument("Error: npts must be positive, got " + std::to_string(npts));
        }
        if (d <= 0)
        {
            throw std::invalid_argument("Error: dimensions must be positive, got " + std::to_string(d));
        }
        if (S.size() != d)
        {
            throw std::invalid_argument("Error: Expected " + std::to_string(d) + " parameter sets, got " + std::to_string(S.size()));
        }

        for (int i = 0; i < d; ++i)
        {
            isValidSobolParams(S[i], i);
        }
    }

    void isValidSobolSequence(const std::vector<std::vector<double>> &P, const int &n0, const int &npts, const int &d)
    {
        if (P.empty())
        {
            throw std::runtime_error("Error: Sobol sequence is empty");
        }
        if (P.size() != npts)
        {
            throw std::runtime_error("Error: Expected " + std::to_string(npts) + " points, got " + std::to_string(P.size()));
        }
        if (P[0].size() != d)
        {
            throw std::runtime_error("Error: Expected " + std::to_string(d) + " dimensions, got " + std::to_string(P[0].size()));
        }

        // Check all values are in [0, 1)
        for (size_t i = 0; i < P.size(); ++i)
        {
            for (size_t j = 0; j < P[i].size(); ++j)
            {
                if (P[i][j] < 0.0 || P[i][j] >= 1.0)
                {
                    throw std::out_of_range("Error: Value " + std::to_string(P[i][j]) + " at point " + std::to_string(i) + ", dimension " + std::to_string(j) + " is outside [0, 1)");
                }
            }
        }
    }
}

// Sobol Sequence Generating Algorithm
std::vector<std::vector<double>> Sobolpts(int n0, int npts, int d, std::vector<Sobolparams> &S)
{
    validateInput(n0, npts, d, S);

    int nmax = npts + n0 - 1;
    int rmax = 1 + floor(log2(nmax));
    int r = (n0 > 1 ? 1 + floor(log2(n0 - 1)) : 1);
    std::vector<std::vector<double>> P(npts, std::vector<double>(d, 0));
    std::vector<std::vector<bool>> y(rmax, std::vector<bool>(d, 0));
    std::vector<std::vector<std::vector<bool>>> V(d, std::vector<std::vector<bool>>(rmax, std::vector<bool>(rmax, 0)));
    int qnext = (1 << r);

    std::bitset<MSB> g((n0 - 1) ^ ((n0 - 1) >> 1));

    for (int i = 0; i < d; ++i)
    {
        Sobolparams s = S[i];
        std::bitset<MSB> c(s.polynomial);
        V[i] = Sobolmat(c, s.q, rmax, s.m_init);
    }

    for (int i = 0; i < d; ++i)
    {
        for (int m = 0; m < rmax; ++m)
        {
            for (int n = 0; n < rmax; ++n)
            {
                y[m][i] = y[m][i] ^ (V[i][m][n] & g[rmax - n - 1]);
            }
        }
    }

    int l;
    for (int k = n0; k < nmax; ++k)
    {
        if (k == qnext)
        {
            r += 1;
            l = 1;
            qnext *= 2;
        }
        else
        {
            l = r - rightmostbit(k - 1);
        }
        for (int i = 0; i < d; ++i)
        {
            for (int m = 0; m < r; ++m)
            {
                y[m][i] = y[m][i] ^ V[i][m][r - l];
            }
            for (int j = 0; j < r; ++j)
            {
                P[k - n0][i] += y[j][i] * pow(2.0, -(j + 1));
            }
        }
    }

    isValidSobolSequence(P, n0, npts, d);

    return P;
}

std::vector<Sobolparams> extractSobolparams(const std::string &filename, int dimensions)
{
    checkFileExists(filename);

    std::ifstream file(filename);
    std::string line;
    std::getline(file, line);
    std::vector<Sobolparams> S;

    while (std::getline(file, line))
    {
        std::stringstream ss(line);
        int d, s, a;
        if (!(ss >> d >> s >> a))
            continue;

        if (d - 1 <= dimensions)
        {
            std::vector<int> mi_vec;
            int value;
            while (ss >> value)
            {
                mi_vec.push_back(value);
            }

            if (mi_vec.empty())
            {
                throw std::invalid_argument("Error: Empty m_i vector at dimension " + std::to_string(d - 1));
            }

            S.push_back({a, s, mi_vec});
        }
        else
        {
            return S;
        }
    }

    if (S.empty())
    {
        throw std::runtime_error("Error: No Sobol parameters loaded from file: " + filename);
    }

    return S;
}
