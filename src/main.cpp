#include <iostream>
#include "particle_grid.h"

int main()
{
    std::vector<Particle> particles = getParticleGrid();
    std::cout << "Generated " << particles.size() << " particles." << std::endl;
    return 0;
}