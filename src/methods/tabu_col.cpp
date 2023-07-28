#include "tabu_col.hpp"

#include <algorithm>
#include <cassert>

#include "../utils/random_generator.hpp"
#include "../utils/utils.hpp"

void tabu_col(Solution &best_solution, const bool verbose) {

    const auto max_time = std::chrono::high_resolution_clock::now() +
                          std::chrono::seconds(Parameters::p->max_time_local_search);

    int64_t best_time = 0;

    if (best_solution.nb_non_empty_colors() < Solution::best_nb_colors) {
        Solution::best_nb_colors = static_cast<int>(best_solution.nb_non_empty_colors());
    }
    std::uniform_int_distribution<int> distribution_tabu(0, 10);
    std::uniform_int_distribution<int> distribution_vertices(0,
                                                             Graph::g->nb_vertices - 1);
    Solution solution(best_solution);
    int turn_main = 0;
    while (not Parameters::p->time_limit_reached_sub_method(max_time) and
           turn_main < Parameters::p->nb_iter_local_search) {

        ++turn_main;
        if (solution.nb_conflicting_vertices() == 0) {
            solution.remove_one_color_and_create_conflicts();
        }
        int best_found = solution.penalty();

        std::vector<std::vector<int>> tabu_matrix(
            Graph::g->nb_vertices, std::vector<int>(solution.nb_colors(), 0));
        long turn = 0;
        while (not Parameters::p->time_limit_reached_sub_method(max_time) and
               best_found != 0) {

            ++turn;

            int best_current = std::numeric_limits<int>::max();
            std::vector<Coloration> best_colorations;

            for (int vertex = 0; vertex < Graph::g->nb_vertices; ++vertex) {
                if (not solution.has_conflicts(vertex)) {
                    continue;
                }
                for (const auto &color : solution.non_empty_colors()) {
                    if (color == solution.color(vertex)) {
                        continue;
                    }
                    const int delta_conflict = solution.delta_conflicts(vertex, color);
                    if (delta_conflict > best_current) {
                        continue;
                    }
                    const bool is_move_tabu = tabu_matrix[vertex][color] >= turn;
                    const bool is_improving =
                        solution.penalty() + delta_conflict < best_found;
                    if (is_move_tabu and not is_improving) {
                        continue;
                    }
                    if (delta_conflict < best_current) {
                        best_current = delta_conflict;
                        best_colorations.clear();
                    }
                    best_colorations.emplace_back(Coloration{vertex, color});
                }
            }
            if (best_colorations.empty()) {
                const int vertex = distribution_vertices(rd::generator);
                // const int vertex = rd::choice(conflicting_vertices);
                int color = rd::choice(solution.non_empty_colors());
                while (color != solution.color(vertex))
                    color = rd::choice(solution.non_empty_colors());
                best_colorations.emplace_back(Coloration{vertex, color});
            }

            const auto [vertex, color] = rd::choice(best_colorations);
            const int old_color = solution.delete_from_color(vertex);
            solution.add_to_color(vertex, color);

            tabu_matrix[vertex][old_color] = static_cast<int>(turn) +
                                             distribution_tabu(rd::generator) +
                                             static_cast<int>(solution.penalty() * 0.6);

            if (solution.penalty() < best_found) {
                best_found = solution.penalty();
                // if (verbose) {
                //     print_result_ls(Parameters::p->elapsed_time(
                //                         std::chrono::high_resolution_clock::now()),
                //                     solution,
                //                     0);
                // }
            }
        }
        if (solution.penalty() == 0) {
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

void tabu_col_neighborhood(Solution &best_solution, const bool verbose) {
    (void)verbose;

    const auto max_time = std::chrono::high_resolution_clock::now() +
                          std::chrono::seconds(Parameters::p->max_time_local_search);

    // int64_t best_time = 0;

    if (best_solution.nb_non_empty_colors() < Solution::best_nb_colors) {
        Solution::best_nb_colors = static_cast<int>(best_solution.nb_non_empty_colors());
    }
    std::uniform_int_distribution<int> distribution_tabu(0, 10);
    std::uniform_int_distribution<int> distribution_vertices(0,
                                                             Graph::g->nb_vertices - 1);
    Solution solution(best_solution);
    int turn_main = 0;

    fmt::print("turn,score,%improve,%regress,%neutral\n");

    while (not Parameters::p->time_limit_reached_sub_method(max_time) and
           turn_main < Parameters::p->nb_iter_local_search) {

        ++turn_main;
        if (solution.nb_conflicting_vertices() == 0) {
            while (solution.non_empty_colors().size() > 47)
                solution.remove_one_color_and_create_conflicts();
        }
        int best_found = solution.penalty();

        int nb_improve = 0;
        int nb_regress = 0;
        int nb_neutral = 0;
        std::vector<std::vector<int>> tabu_matrix(
            Graph::g->nb_vertices, std::vector<int>(solution.nb_colors(), 0));
        long turn = 0;
        while (not Parameters::p->time_limit_reached_sub_method(max_time) and
               best_found != 0) {

            ++turn;

            int best_current = std::numeric_limits<int>::max();
            std::vector<Coloration> best_colorations;

            for (int vertex = 0; vertex < Graph::g->nb_vertices; ++vertex) {
                if (not solution.has_conflicts(vertex)) {
                    continue;
                }
                for (const auto &color : solution.non_empty_colors()) {
                    if (color == solution.color(vertex)) {
                        continue;
                    }
                    const int delta_conflict = solution.delta_conflicts(vertex, color);

                    if (delta_conflict == 0) {
                        ++nb_neutral;
                    } else if (delta_conflict > 0) {
                        ++nb_regress;
                    } else {
                        ++nb_improve;
                    }

                    if (delta_conflict > best_current) {
                        continue;
                    }
                    const bool is_move_tabu = tabu_matrix[vertex][color] >= turn;
                    const bool is_improving =
                        solution.penalty() + delta_conflict < best_found;
                    if (is_move_tabu and not is_improving) {
                        continue;
                    }
                    if (delta_conflict < best_current) {
                        best_current = delta_conflict;
                        best_colorations.clear();
                    }
                    best_colorations.emplace_back(Coloration{vertex, color});
                }
            }
            if (best_colorations.empty()) {
                const int vertex = distribution_vertices(rd::generator);
                // const int vertex = rd::choice(conflicting_vertices);
                int color = rd::choice(solution.non_empty_colors());
                while (color != solution.color(vertex))
                    color = rd::choice(solution.non_empty_colors());
                best_colorations.emplace_back(Coloration{vertex, color});
            }

            const float total = static_cast<float>(nb_improve + nb_regress + nb_neutral);
            fmt::print("{},{},{},{},{}\n",
                       turn,
                       solution.penalty(),
                       static_cast<float>(nb_improve) / total,
                       static_cast<float>(nb_regress) / total,
                       static_cast<float>(nb_neutral) / total);
            const auto [vertex, color] = rd::choice(best_colorations);
            const int old_color = solution.delete_from_color(vertex);
            solution.add_to_color(vertex, color);

            tabu_matrix[vertex][old_color] = static_cast<int>(turn) +
                                             distribution_tabu(rd::generator) +
                                             static_cast<int>(solution.penalty() * 0.6);

            if (solution.penalty() < best_found) {
                best_found = solution.penalty();
                // if (verbose) {
                //     print_result_ls(Parameters::p->elapsed_time(
                //                         std::chrono::high_resolution_clock::now()),
                //                     solution,
                //                     0);
                // }
            }
        }
        if (solution.penalty() == 0) {
            best_solution = solution;
            if (solution.nb_non_empty_colors() < Solution::best_nb_colors) {
                Solution::best_nb_colors =
                    static_cast<int>(solution.nb_non_empty_colors());
            }
            // if (verbose) {
            //     best_time = Parameters::p->elapsed_time(
            //         std::chrono::high_resolution_clock::now());
            //     //     print_result_ls(best_time, solution, turn);
            // }
        }
    }
    // if (verbose) {
    //     print_result_ls(best_time, best_solution, turn_main);
    // }
}

void random_walk_gcp(Solution &best_solution, const bool verbose) {
    (void)verbose;
    const auto max_time = std::chrono::high_resolution_clock::now() +
                          std::chrono::seconds(Parameters::p->max_time_local_search);

    // int64_t best_time = 0;

    if (best_solution.nb_non_empty_colors() < Solution::best_nb_colors) {
        Solution::best_nb_colors = static_cast<int>(best_solution.nb_non_empty_colors());
    }
    std::uniform_int_distribution<int> distribution_tabu(0, 10);
    std::uniform_int_distribution<int> distribution_vertices(0,
                                                             Graph::g->nb_vertices - 1);
    Solution solution(best_solution);
    int turn_main = 0;

    fmt::print("turn,score,nb_neighbors,{}\n", Solution::header_csv);
    // fmt::print("turn,score,%improve,%regress,%neutral\n");

    while (not Parameters::p->time_limit_reached_sub_method(max_time) and turn_main < 1) {

        ++turn_main;
        if (solution.nb_conflicting_vertices() == 0) {
            while (solution.non_empty_colors().size() > 47)
                solution.remove_one_color_and_create_conflicts();
        }
        int best_found = solution.penalty();

        int nb_improve = 0;
        int nb_regress = 0;
        int nb_neutral = 0;
        std::vector<std::vector<int>> tabu_matrix(
            Graph::g->nb_vertices, std::vector<int>(solution.nb_colors(), 0));
        long turn = 0;
        while (not Parameters::p->time_limit_reached_sub_method(max_time) and
               best_found != 0 and turn < Parameters::p->nb_iter_local_search) {

            ++turn;

            // int best_current = std::numeric_limits<int>::max();
            std::vector<Coloration> best_colorations;

            for (int vertex = 0; vertex < Graph::g->nb_vertices; ++vertex) {
                if (not solution.has_conflicts(vertex)) {
                    continue;
                }
                for (const auto &color : solution.non_empty_colors()) {
                    if (color == solution.color(vertex)) {
                        continue;
                    }
                    const int delta_conflict = solution.delta_conflicts(vertex, color);

                    if (delta_conflict == 0) {
                        ++nb_neutral;
                    } else if (delta_conflict > 0) {
                        ++nb_regress;
                    } else {
                        ++nb_improve;
                    }
                    best_colorations.emplace_back(Coloration{vertex, color});
                }
            }
            if (best_colorations.empty()) {
                const int vertex = distribution_vertices(rd::generator);
                // const int vertex = rd::choice(conflicting_vertices);
                int color = rd::choice(solution.non_empty_colors());
                while (color != solution.color(vertex))
                    color = rd::choice(solution.non_empty_colors());
                best_colorations.emplace_back(Coloration{vertex, color});
            }

            // const float total = static_cast<float>(nb_improve + nb_regress +
            // nb_neutral); fmt::print("{},{},{},{},{}\n",
            //            turn,
            //            solution.penalty(),
            //            static_cast<float>(nb_improve) / total,
            //            static_cast<float>(nb_regress) / total,
            //            static_cast<float>(nb_neutral) / total);
            fmt::print("{},{},{},{}\n",
                       turn,
                       solution.penalty(),
                       nb_improve + nb_regress + nb_neutral,
                       solution.line_csv());
            const auto [vertex, color] = rd::choice(best_colorations);
            const int old_color = solution.delete_from_color(vertex);
            solution.add_to_color(vertex, color);

            tabu_matrix[vertex][old_color] = static_cast<int>(turn) +
                                             distribution_tabu(rd::generator) +
                                             static_cast<int>(solution.penalty() * 0.6);

            if (solution.penalty() < best_found) {
                best_found = solution.penalty();
                // if (verbose) {
                //     print_result_ls(Parameters::p->elapsed_time(
                //                         std::chrono::high_resolution_clock::now()),
                //                     solution,
                //                     0);
                // }
            }
        }
        if (solution.penalty() == 0) {
            best_solution = solution;
            if (solution.nb_non_empty_colors() < Solution::best_nb_colors) {
                Solution::best_nb_colors =
                    static_cast<int>(solution.nb_non_empty_colors());
            }
            // if (verbose) {
            //     best_time = Parameters::p->elapsed_time(
            //         std::chrono::high_resolution_clock::now());
            //     print_result_ls(best_time, solution, turn);
            // }
        }
    }
    // if (verbose) {
    //     print_result_ls(best_time, best_solution, turn_main);
    // }
}
