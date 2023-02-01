#pragma once

#include "../representation/Solution.hpp"

/**
 * @brief Local search that pass
 *
 * @param solution solution to use, the solution will be modified but stay legal
 * @param verbose True if print csv line each time new best scores is found
 */
void none_ls(Solution &solution, const bool verbose = false);
