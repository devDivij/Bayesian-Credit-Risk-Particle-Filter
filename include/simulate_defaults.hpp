#pragma once
#include <vector>
#include <string>

std::vector<std::vector<double>> genParticleLossECDFs(const std::vector<std::vector<double>> &PDs);

std::vector<double> genTailConcentratedBins();

void saveECDFs(const std::string &filename, const std::vector<std::vector<double>> &data);

void saveBins(const std::string &filename, const std::vector<double> bins);