#include "worst_ls.hpp"

#include "../utils/random_generator.hpp"
#include "../utils/utils.hpp"

void worst_ls(Solution &best_solution, const bool verbose) {

    for (int vertex = 0; vertex < Graph::g->nb_vertices; ++vertex) {
        best_solution.delete_from_color(vertex);
    }

    std::uniform_int_distribution<int> distribution(0, 100);

    for (int vertex = 0; vertex < Graph::g->nb_vertices; ++vertex) {
        if (distribution(rd::generator) < 10) {
            const auto possible_colors = best_solution.available_colors(vertex);
            best_solution.add_to_color(vertex, rd::choice(possible_colors));
        } else {
            best_solution.add_to_color(vertex, -1);
        }
    }

    int64_t best_time = 0;
    if (verbose) {
        best_time =
            Parameters::p->elapsed_time(std::chrono::high_resolution_clock::now());
        print_result_ls(best_time, best_solution, 1);
        print_result_ls(best_time, best_solution, 1);
    }
}
