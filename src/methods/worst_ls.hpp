#pragma once

#include "../representation/Solution.hpp"

/**
 * @brief Local search that transform the solution on the worst possible one
 *
 * Each vertex as its own color
 *
 * @param solution solution to use, the solution will be modified but stay legal
 * @param verbose True if print csv line each time new best scores is found
 */
void worst_ls(Solution &solution, const bool verbose = false);
