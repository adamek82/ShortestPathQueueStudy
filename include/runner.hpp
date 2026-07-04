#pragma once

#include "search.hpp"

#include <string>

namespace spq {

SearchMode parseSearchMode(const std::string& mode);
const char* searchModeName(SearchMode mode);

int detectHardwareConcurrency();
int resolveThreadCount(int requestedThreads);

SearchResult runSearchWorker(SearchMode mode, AnnealingConfig config);
SearchResult runParallel(SearchMode mode, AnnealingConfig config, int requestedThreads);

} // namespace spq
