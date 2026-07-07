#include "eae_firmware/CliOptions.h"

#include <stdexcept>

namespace eae::firmware {

namespace {

double parseDouble(const std::vector<std::string>& args, std::size_t& index) {
    if (index + 1 >= args.size()) {
        throw std::invalid_argument("Missing value after " + args[index]);
    }
    ++index;
    return std::stod(args[index]);
}

int parseInt(const std::vector<std::string>& args, std::size_t& index) {
    if (index + 1 >= args.size()) {
        throw std::invalid_argument("Missing value after " + args[index]);
    }
    ++index;
    return std::stoi(args[index]);
}

bool parseBool(const std::vector<std::string>& args, std::size_t& index) {
    return parseInt(args, index) != 0;
}

}  
// namespace

CliOptions parseCliOptions(int argc, char* argv[]) {
    CliOptions options;
    std::vector<std::string> args(argv + 1, argv + argc);

    for (std::size_t i = 0; i < args.size(); ++i) {
        const auto& arg = args[i];

        if (arg == "--help" || arg == "-h") {
            throw std::invalid_argument(usageText());
        }
        if (arg == "--target-temp") {
            options.targetTempC = parseDouble(args, i);
        } else if (arg == "--initial-temp") {
            options.initialTempC = parseDouble(args, i);
        } else if (arg == "--ambient-temp") {
            options.ambientTempC = parseDouble(args, i);
        } else if (arg == "--heat-load") {
            options.heatLoadKw = parseDouble(args, i);
        } else if (arg == "--dt") {
            options.dtSeconds = parseDouble(args, i);
        } else if (arg == "--steps") {
            options.steps = parseInt(args, i);
        } else if (arg == "--coolant-ok") {
            options.coolantOk = parseBool(args, i);
        } else if (arg == "--ignition") {
            options.ignitionOn = parseBool(args, i);
        } else if (arg == "--pump-fault") {
            options.pumpFault = parseBool(args, i);
        } else {
            throw std::invalid_argument("Unknown argument: " + arg + "\n" + usageText());
        }
    }

    if (options.dtSeconds <= 0.0) {
        throw std::invalid_argument("--dt must be greater than zero");
    }
    if (options.steps <= 0) {
        throw std::invalid_argument("--steps must be greater than zero");
    }

    return options;
}

std::string usageText() {
    return "Usage: eae_firmware [--target-temp C] [--initial-temp C] "
           "[--ambient-temp C] [--heat-load kW] [--dt seconds] [--steps N] "
           "[--coolant-ok 0|1] [--ignition 0|1] [--pump-fault 0|1]";
}

}  // namespace eae::firmware
