#include "partial_col.hpp"

#include <algorithm>
#include <assert.h>
#include <numeric>
#include <random>
#include <set>
#include <vector>

#include "../utils/random_generator.hpp"
#include "../utils/utils.hpp"

std::set<int> remove_color_unassigned(Solution &solution) {
    // look for the best color group to relocate them in an
    // other color without having too much unassigned vertices
    int best_sum_conflicts = Graph::g->nb_vertices;
    int best_color1 = -1;
    int best_color2 = -1;
    for (const auto &color1 : solution.non_empty_colors()) {
        for (const auto &color2 : solution.non_empty_colors()) {
            if (color1 == color2) {
                continue;
            }
            int sum_conflicts{0};
            for (const int vertex : solution.colors_vertices(color1)) {
                sum_conflicts += solution.conflicts_colors(color2, vertex);
            }
            if (sum_conflicts < best_sum_conflicts) {
                best_color1 = color1;
                best_color2 = color2;
                best_sum_conflicts = sum_conflicts;
            }
        }
    }
    // delete the vertices in the color and try to relocate them in the second color
    // if not possible add them to a unassigned set
    std::set<int> unassigned;
    const auto to_delete = solution.colors_vertices(best_color1);
    for (const auto vertex : to_delete) {
        solution.delete_from_color(vertex);
        if (solution.conflicts_colors(best_color2, vertex) == 0) {
            solution.add_to_color(vertex, best_color2);
        } else {
            unassigned.insert(vertex);
        }
    }
    return unassigned;
}

void partial_col(Solution &best_solution, const bool verbose) {
    const auto max_time = std::chrono::high_resolution_clock::now() +
                          std::chrono::seconds(Parameters::p->max_time_local_search);

    int64_t best_time = 0;

    if (best_solution.nb_non_empty_colors() < Solution::best_nb_colors) {
        Solution::best_nb_colors = static_cast<int>(best_solution.nb_non_empty_colors());
    }
    std::uniform_int_distribution<int> distribution(0, 10);
    Solution solution(best_solution);
    int turn_main = 0;
    while (not Parameters::p->time_limit_reached_sub_method(max_time) and
           turn_main < Parameters::p->nb_iter_local_search) {

        ++turn_main;

        std::set<int> unassigned = remove_color_unassigned(solution);
        int best_found = static_cast<int>(unassigned.size());

        std::vector<std::vector<long>> tabu_matrix(
            Graph::g->nb_vertices, std::vector<long>(solution.nb_colors(), 0));
        long turn = 0;
        while (not Parameters::p->time_limit_reached_sub_method(max_time) and
               best_found != 0) {

            ++turn;

            int best_current = std::numeric_limits<int>::max();
            std::vector<Coloration> best_colorations;

            for (const int vertex : unassigned) {
                for (const int color : solution.non_empty_colors()) {
                    const int nb_conflicts = solution.conflicts_colors(color, vertex);
                    if (nb_conflicts > best_current) {
                        continue;
                    }
                    const bool is_move_tabu = tabu_matrix[vertex][color] >= turn;
                    const bool is_improving =
                        nb_conflicts == 0 and
                        static_cast<int>(unassigned.size()) <= best_found;
                    if (is_move_tabu and not is_improving) {
                        continue;
                    }

                    if (nb_conflicts < best_current) {
                        best_current = nb_conflicts;
                        best_colorations.clear();
                    }
                    best_colorations.emplace_back(Coloration{vertex, color});
                }
            }

            // If no move, pick a random one
            if (best_colorations.empty()) {
                const int vertex = rd::choice(unassigned);
                const int color = rd::choice(solution.non_empty_colors());
                best_colorations.emplace_back(Coloration{vertex, color});
            }

            Coloration chosen_one = rd::choice(best_colorations);
            solution.add_to_color(chosen_one.vertex, chosen_one.color);
            unassigned.erase(chosen_one.vertex);

            // Remove conflicting nodes
            for (const int neighbor : Graph::g->neighborhood[chosen_one.vertex]) {
                // Do not move neighbors to best_color for a couple of iterations in order
                // to avoid best_vertex from dropping back out too soon
                long t_tenure =
                    static_cast<int>(0.6 * static_cast<double>(unassigned.size())) +
                    distribution(rd::generator);
                tabu_matrix[neighbor][chosen_one.color] = turn + t_tenure;

                // Check for conflict created by moving best_vertex to best_color
                if (solution.color(neighbor) == chosen_one.color) {
                    solution.delete_from_color(neighbor);
                    unassigned.insert(neighbor);
                }
            }

            const int nb_unassigned = static_cast<int>(unassigned.size());
            if (nb_unassigned < best_found) {
                best_found = nb_unassigned;
                // if (verbose) {
                //     best_time = Parameters::p->elapsed_time(
                //         std::chrono::high_resolution_clock::now());
                //     print_result_ls(best_time, solution, turn);
                // }
            }
        }

        if (unassigned.empty()) {
            best_solution = solution;
            if (solution.nb_non_empty_colors() < Solution::best_nb_colors) {
                Solution::best_nb_colors =
                    static_cast<int>(solution.nb_non_empty_colors());
            }
            if (verbose) {
                best_time = Parameters::p->elapsed_time(
                    std::chrono::high_resolution_clock::now());
                print_result_ls(best_time, solution, turn);
            }
        }
    }
    if (verbose) {
        print_result_ls(best_time, best_solution, turn_main);
    }
}
