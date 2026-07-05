#pragma once

#include <string>
#include <vector>

namespace eae::firmware {

struct CliOptions {
    double targetTempC{65.0};
    double initialTempC{42.0};
    double ambientTempC{25.0};
    double heatLoadKw{12.0};
    double dtSeconds{1.0};
    int steps{90};
    bool coolantOk{true};
    bool ignitionOn{true};
    bool pumpFault{false};
};

CliOptions parseCliOptions(int argc, char* argv[]);
std::string usageText();

}  // namespace eae::firmware
