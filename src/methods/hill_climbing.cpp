#include "hill_climbing.hpp"

#include "../utils/random_generator.hpp"
#include "../utils/utils.hpp"

void hill_climbing_one_move(Solution &solution, const bool verbose) {
    int64_t best_time = 0;
    long turn = 0;
    while (not Parameters::p->time_limit_reached() and
           solution.score_wvcp() != Parameters::p->target) {
        ++turn;
        std::vector<Coloration> best_coloration;
        int best_evaluation = solution.score_wvcp();
        for (int vertex = 0; vertex < Graph::g->nb_vertices; ++vertex) {
            for (const auto color : solution.non_empty_colors()) {
                if (color == solution.color(vertex) or
                    solution.conflicts_colors(color, vertex) != 0) {
                    continue;
                }
                const int test_score =
                    solution.score_wvcp() + solution.delta_wvcp_score(vertex, color);
                if (test_score < best_evaluation) {
                    best_coloration.clear();
                    best_coloration.emplace_back(Coloration{vertex, color});
                    best_evaluation = test_score;
                } else if (test_score == best_evaluation and
                           not best_coloration.empty()) {
                    best_coloration.emplace_back(Coloration{vertex, color});
                }
            }
        }
        if (best_coloration.empty()) {
            return;
        }
        const Coloration chosen_one = rd::choice(best_coloration);
        solution.delete_from_color(chosen_one.vertex);
        solution.add_to_color(chosen_one.vertex, chosen_one.color);
        if (verbose) {
            best_time =
                Parameters::p->elapsed_time(std::chrono::high_resolution_clock::now());
            print_result_ls(best_time, solution, turn);
        }
    }
}