#pragma once

#include "../representation/ProxiSolutionRedLS.hpp"

/**
 * @brief Local search from redLS
 *
 * From :
 * Wang, Y., Cai, S., Pan, S., Li, X., Yin, M., 2020.
 * Reduction and Local Search for Weighted Graph Coloring Problem.
 * AAAI 34, 2433–2441.
 * https://doi.org/10.1609/aaai.v34i03.5624
 *
 * @param solution solution to use, the solution will be modified
 * @param verbose True if print csv line each time new best scores is found
 */
void redls_freeze(Solution &solution, const bool verbose = false);

/**
 * @brief Apply candidate moves set 1 (RedLS)
 *
 *
 * @param solution solution
 * @param best_local_score best score found in the local search
 * @return bool true if a move have been applied
 */
bool improve_conflicts_and_score_freeze(ProxiSolutionRedLS &solution,
                                        const int best_local_score,
                                        std::vector<bool> &tabu_list);

/**
 * @brief Apply candidate moves set 2 (RedLS)
 *
 * @param solution solution to use, the solution will be modified
 * @param with_conf boolean flag indicating if the tabu list is activated
 * @return bool true if a move have been applied
 */
bool improve_conflicts_freeze(ProxiSolutionRedLS &solution,
                              const bool with_conf,
                              std::vector<bool> &tabu_list);

/**
 * @brief Apply candidate moves set 3 (RedLS)
 *
 * @param solution solution to use, the solution will be modified
 * @param best_local_score best score found in the local search
 * @return bool true if a move have been applied
 */
bool solve_one_conflict_preserve_score_freeze(ProxiSolutionRedLS &solution,
                                              const int best_local_score,
                                              std::vector<bool> &tabu_list);

/**
 * @brief Apply list of moves selected by Rule 1 (RedLS)
 *
 * @param solution solution to use, the solution will be modified
 * @return bool false if no possible move
 */
bool move_heaviest_vertices_freeze(ProxiSolutionRedLS &solution);

/**
 * @brief Apply list of moves selected by Rule 2 (RedLS)
 * @return bool false if no possible move
 */
bool solve_one_conflict_freeze(ProxiSolutionRedLS &solution,
                               const int best_local_score,
                               std::vector<bool> &tabu_list);
