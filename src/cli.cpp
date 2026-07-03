#include "cli.hpp"

#include <algorithm>
#include <ostream>
#include <stdexcept>
#include <string>

namespace spq {
namespace {

std::string lower(std::string value) {
    std::transform(value.begin(), value.end(), value.begin(),
                   [](unsigned char ch) { return static_cast<char>(std::tolower(ch)); });
    return value;
}

std::string takeValue(int& i, int argc, char** argv, const std::string& option) {
    if (i + 1 >= argc)
        throw std::runtime_error("Missing value for " + option);

    ++i;
    return argv[i];
}

bool parseBool(const std::string& value) {
    const std::string v = lower(value);

    if (v == "1" || v == "true" || v == "yes" || v == "on")
        return true;

    if (v == "0" || v == "false" || v == "no" || v == "off")
        return false;

    throw std::runtime_error("Invalid boolean value: " + value);
}

} // namespace

ProgramOptions parseArgs(int argc, char** argv) {
    ProgramOptions options;

    for (int i = 1; i < argc; ++i) {
        const std::string arg = argv[i];

        if (arg == "--help" || arg == "-h") {
            options.help = true;
        } else if (arg == "--mode") {
            options.mode = lower(takeValue(i, argc, argv, arg));
        } else if (arg == "--n") {
            options.n = std::stoi(takeValue(i, argc, argv, arg));
        } else if (arg == "--iterations") {
            options.iterations = std::stoi(takeValue(i, argc, argv, arg));
        } else if (arg == "--restarts") {
            options.restarts = std::stoi(takeValue(i, argc, argv, arg));
        } else if (arg == "--density") {
            options.density = std::stod(takeValue(i, argc, argv, arg));
        } else if (arg == "--seed") {
            options.seed = static_cast<unsigned int>(
                std::stoul(takeValue(i, argc, argv, arg)));
        } else if (arg == "--progress-every") {
            options.progressEvery = std::stoi(takeValue(i, argc, argv, arg));
        } else if (arg == "--require-reachable") {
            options.requireReachable = parseBool(takeValue(i, argc, argv, arg));
        } else if (arg == "--allow-unreachable") {
            options.requireReachable = false;
        } else if (arg == "--start-temp") {
            options.startTemperature = std::stod(takeValue(i, argc, argv, arg));
        } else if (arg == "--end-temp") {
            options.endTemperature = std::stod(takeValue(i, argc, argv, arg));
        } else if (arg == "--quiet") {
            options.verbose = false;
        } else if (arg == "--verbose") {
            options.verbose = true;
        } else {
            throw std::runtime_error("Unknown argument: " + arg);
        }
    }

    return options;
}

SearchConfig makeSearchConfig(const ProgramOptions& options) {
    SearchConfig config;
    config.n = options.n;
    config.iterations = options.iterations;
    config.restarts = options.restarts;
    config.initialBlockProbability = options.density;
    config.requireReachable = options.requireReachable;
    config.progressEvery = options.progressEvery;
    config.seed = options.seed;
    config.verbose = options.verbose;
    return config;
}

AnnealingConfig makeAnnealingConfig(const ProgramOptions& options) {
    AnnealingConfig config;
    config.n = options.n;
    config.iterations = options.iterations;
    config.restarts = options.restarts;
    config.initialBlockProbability = options.density;
    config.requireReachable = options.requireReachable;
    config.progressEvery = options.progressEvery;
    config.seed = options.seed;
    config.verbose = options.verbose;
    config.startTemperature = options.startTemperature;
    config.endTemperature = options.endTemperature;
    return config;
}

void printUsage(std::ostream& out, const char* programName) {
    out << "Usage:\n"
        << "  " << programName << " [options]\n\n"
        << "Modes:\n"
        << "  --mode ref       Run the built-in 20x20 reference grid. Default.\n"
        << "  --mode random    Random independent grids.\n"
        << "  --mode hill      Hill climbing.\n"
        << "  --mode anneal    Simulated annealing.\n\n"
        << "Common options:\n"
        << "  --n N                         Grid size. Default: 20\n"
        << "  --iterations N                Iterations per restart. Default: 100000\n"
        << "  --restarts N                  Number of restarts. Default: 10\n"
        << "  --density P                   Initial obstacle probability. Default: 0.35\n"
        << "  --seed N                      RNG seed. Default: random_device\n"
        << "  --progress-every N            Progress print interval. Default: 10000\n"
        << "  --require-reachable true|false\n"
        << "  --allow-unreachable           Shortcut for --require-reachable false\n"
        << "  --quiet                       Disable progress logs\n\n"
        << "Annealing options:\n"
        << "  --start-temp T                Default: 5.0\n"
        << "  --end-temp T                  Default: 0.01\n\n"
        << "Examples:\n"
        << "  " << programName << " --mode ref\n"
        << "  " << programName << " --mode hill --n 20 --iterations 200000 --seed 123\n"
        << "  " << programName
        << " --mode anneal --n 30 --iterations 500000 --density 0.35\n";
}

} // namespace spq
