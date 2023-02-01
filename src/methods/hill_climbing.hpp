#pragma once

#include "../representation/Solution.hpp"

/**
 * @brief Hill climbing with a one move operator, will stop when reach a best local score
 *
 * @param solution solution to use, the solution will be modified
 * @param verbose True if print csv line each time new best scores is found
 */
void hill_climbing_one_move(Solution &solution, const bool verbose = false);
