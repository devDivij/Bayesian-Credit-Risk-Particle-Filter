#pragma once
#include <vector>
#include <string>
#include <iostream>


// exposure_class
// {
//     RETAIL=0,
//     CORPORATE=1,
//     INSTITUTIONS=2,
//     REALESTATE=3,
//     SOVEREIGN=4
// };



struct Debtor
{
    int id;
    double feature_1;
    double feature_2;
    double feature_3;
    double feature_4;
    int exposure_class;
};

std::vector<std::vector<double>> genPDs();
void savePDs(const std::string& filename, const std::vector<std::vector<double>>& data);