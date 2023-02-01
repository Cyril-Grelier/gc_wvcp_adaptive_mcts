#pragma once

#include "../representation/Solution.hpp"

/**
 * @brief Tabu search with a one move operator inspired from TabuCol but considering
 * weights of the graph
 *
 * @param solution solution to use, the solution will be modified
 * @param verbose True if print csv line each time new best scores is found
 */
void tabu_weight(Solution &solution, const bool verbose = false);
