#pragma once

#include "search.hpp"

#include <iosfwd>
#include <string>

namespace spq {

struct ProgramOptions {
    std::string mode{"ref"};

    int n{20};
    int iterations{100000};
    int restarts{10};
    double density{0.35};
    bool requireReachable{true};
    int progressEvery{10000};
    unsigned int seed{0};
    bool verbose{true};

    double startTemperature{5.0};
    double endTemperature{0.01};

    bool help{false};
};

ProgramOptions parseArgs(int argc, char** argv);

SearchConfig makeSearchConfig(const ProgramOptions& options);
AnnealingConfig makeAnnealingConfig(const ProgramOptions& options);

void printUsage(std::ostream& out, const char* programName);

} // namespace spq
