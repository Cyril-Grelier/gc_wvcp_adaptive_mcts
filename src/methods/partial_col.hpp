#pragma once

#include "../representation/Solution.hpp"

/**
 * @brief partial_col
 *
 * @param solution solution to use, the solution will be modified
 * @param verbose True if print csv line each time new best scores is found
 */
void partial_col(Solution &solution, const bool verbose = false);
