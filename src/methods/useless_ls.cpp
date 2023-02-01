#include "useless_ls.hpp"

#include "../utils/random_generator.hpp"
#include "../utils/utils.hpp"

void useless_ls(Solution &best_solution, const bool verbose) {
    int64_t best_time = 0;

    auto max_time = std::chrono::high_resolution_clock::now() +
                    std::chrono::seconds(Parameters::p->max_time_local_search);
    // just wait doing nothing
    while (not Parameters::p->time_limit_reached_sub_method(max_time)) {
        const int color = best_solution.delete_from_color(0);
        best_solution.add_to_color(0, color);
    }

    if (verbose) {
        best_time =
            Parameters::p->elapsed_time(std::chrono::high_resolution_clock::now());
        print_result_ls(best_time, best_solution, 1);
        print_result_ls(best_time, best_solution, 1);
    }
}
