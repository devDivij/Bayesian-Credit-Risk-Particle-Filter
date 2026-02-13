#pragma once
#include <vector>
#include <string>
#include <iostream>
#include "model_config.hpp"

std::vector<std::vector<double>> genPDs(const std::vector<Particle> &particles);
void savePDs(const std::string &filename, const std::vector<std::vector<double>> &data);