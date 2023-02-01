#pragma once

#include "../representation/Solution.hpp"

/**
 * @brief afisa main algorithm
 *
 * From :
 * Sun, W., Hao, J.-K., Lai, X., Wu, Q., 2018.
 * Adaptive feasible and infeasible tabu search for weighted vertex coloring.
 * Information Sciences 466, 203–219. https://doi.org/10.1016/j.ins.2018.07.037
 *
 * @param solution solution to use, the solution will be modified but stay legal
 * @param verbose True if print csv line each time new best scores is found
 */
void afisa_original(Solution &solution, const bool verbose = false);

/**
 * @brief Tabu search for afisa
 *
 * @param solution current solution (may not be legal)
 * @param best_solution ref solution for aspiration criteria (legal)
 * @param best_afisa_sol best afisa solution (may not be legal)
 * @param penalty_coeff penalty coefficient
 * @param turns number of turns of tabu
 * @param perturbation type of perturbation
 * @param max_time time limit for the local search
 */
void afisa_original_tabu(Solution &solution,
                         const Solution &best_solution,
                         Solution &best_afisa_sol,
                         const int &penalty_coeff,
                         const long &turns,
                         const Perturbation &perturbation,
                         const std::chrono::high_resolution_clock::time_point &max_time);