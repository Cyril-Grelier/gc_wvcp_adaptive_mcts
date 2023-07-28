#include "tabu_weight.hpp"

#include "../utils/random_generator.hpp"
#include "../utils/utils.hpp"

void tabu_weight(Solution &best_solution, const bool verbose) {

    auto max_time = std::chrono::high_resolution_clock::now() +
                    std::chrono::seconds(Parameters::p->max_time_local_search);

    int64_t best_time = 0;
    std::vector<long> tabu_list(Graph::g->nb_vertices, 0);
    Solution solution = best_solution;
    long turn = 0;
    while (not Parameters::p->time_limit_reached_sub_method(max_time) and
           turn < Parameters::p->nb_iter_local_search and
           best_solution.score_wvcp() != Parameters::p->target) {
        ++turn;
        std::vector<Coloration> best_coloration;
        int best_evaluation = std::numeric_limits<int>::max();
        auto possible_colors = solution.non_empty_colors();
        possible_colors.push_back(-1);
        for (int vertex = 0; vertex < Graph::g->nb_vertices; ++vertex) {
            for (const int &color : possible_colors) {
                if (color == solution.color(vertex) or
                    (color != -1 and solution.conflicts_colors(color, vertex) != 0)) {
                    continue;
                }
                const int test_score =
                    solution.score_wvcp() + solution.delta_wvcp_score(vertex, color);
                if ((test_score < best_evaluation and tabu_list[vertex] <= turn) or
                    (test_score < best_solution.score_wvcp())) {
                    best_coloration.clear();
                    best_coloration.emplace_back(Coloration{vertex, color});
                    best_evaluation = test_score;
                } else if (test_score == best_evaluation and
                           (tabu_list[vertex] <= turn or
                            test_score < best_solution.score_wvcp())) {
                    best_coloration.emplace_back(Coloration{vertex, color});
                }
            }
        }
        if (not best_coloration.empty()) {
            const Coloration chosen_one = rd::choice(best_coloration);
            solution.delete_from_color(chosen_one.vertex);
            solution.add_to_color(chosen_one.vertex, chosen_one.color);
            tabu_list[chosen_one.vertex] = turn + solution.nb_non_empty_colors();
            if (solution.score_wvcp() < best_solution.score_wvcp()) {
                best_solution = solution;
                if (verbose) {
                    best_time = Parameters::p->elapsed_time(
                        std::chrono::high_resolution_clock::now());
                    print_result_ls(best_time, best_solution, turn);
                }
            } else if (solution.score_wvcp() == best_solution.score_wvcp()) {
                best_solution = solution;
            }
        }
    }
    if (verbose) {
        print_result_ls(best_time, best_solution, turn);
    }
}

void tabu_weight_neighborhood(Solution &best_solution, const bool verbose) {
    (void)verbose;
    auto max_time = std::chrono::high_resolution_clock::now() +
                    std::chrono::seconds(Parameters::p->max_time_local_search);

    // int64_t best_time = 0;
    std::vector<long> tabu_list(Graph::g->nb_vertices, 0);
    Solution solution = best_solution;
    long turn = 0;
    fmt::print("turn,score,%improve,%regress,%neutral\n");

    while (not Parameters::p->time_limit_reached_sub_method(max_time) and
           turn < Parameters::p->nb_iter_local_search and
           best_solution.score_wvcp() != Parameters::p->target) {
        ++turn;
        std::vector<Coloration> best_coloration;
        int best_evaluation = std::numeric_limits<int>::max();
        auto possible_colors = solution.non_empty_colors();
        possible_colors.push_back(-1);

        int nb_improve = 0;
        int nb_regress = 0;
        int nb_neutral = 0;

        for (int vertex = 0; vertex < Graph::g->nb_vertices; ++vertex) {
            for (const int &color : possible_colors) {
                if (color == solution.color(vertex) or
                    (color != -1 and solution.conflicts_colors(color, vertex) != 0)) {
                    continue;
                }
                const int delta = solution.delta_wvcp_score(vertex, color);
                const int test_score = solution.score_wvcp() + delta;

                if (delta == 0) {
                    ++nb_neutral;
                } else if (delta > 0) {
                    ++nb_regress;
                } else {
                    ++nb_improve;
                }
                if ((test_score < best_evaluation and tabu_list[vertex] <= turn) or
                    (test_score < best_solution.score_wvcp())) {
                    best_coloration.clear();
                    best_coloration.emplace_back(Coloration{vertex, color});
                    best_evaluation = test_score;
                } else if (test_score == best_evaluation and
                           (tabu_list[vertex] <= turn or
                            test_score < best_solution.score_wvcp())) {
                    best_coloration.emplace_back(Coloration{vertex, color});
                }
            }
        }
        if (not best_coloration.empty()) {
            const float total = static_cast<float>(nb_improve + nb_regress + nb_neutral);
            fmt::print("{},{},{},{},{}\n",
                       turn,
                       solution.score_wvcp(),
                       static_cast<float>(nb_improve) / total,
                       static_cast<float>(nb_regress) / total,
                       static_cast<float>(nb_neutral) / total);

            const Coloration chosen_one = rd::choice(best_coloration);
            solution.delete_from_color(chosen_one.vertex);
            solution.add_to_color(chosen_one.vertex, chosen_one.color);
            tabu_list[chosen_one.vertex] = turn + solution.nb_non_empty_colors();
            if (solution.score_wvcp() < best_solution.score_wvcp()) {
                best_solution = solution;
                // if (verbose) {
                //     best_time = Parameters::p->elapsed_time(
                //         std::chrono::high_resolution_clock::now());
                //     print_result_ls(best_time, best_solution, turn);
                // }
            } else if (solution.score_wvcp() == best_solution.score_wvcp()) {
                best_solution = solution;
            }
        }
    }
    // if (verbose) {
    //     print_result_ls(best_time, best_solution, turn);
    // }
}

void random_walk_wvcp(Solution &best_solution, const bool verbose) {
    (void)verbose;

    auto max_time = std::chrono::high_resolution_clock::now() +
                    std::chrono::seconds(Parameters::p->max_time_local_search);

    // int64_t best_time = 0;
    std::vector<long> tabu_list(Graph::g->nb_vertices, 0);
    Solution solution = best_solution;
    long turn = 0;
    fmt::print("turn,score,%improve,%regress,%neutral\n");

    while (not Parameters::p->time_limit_reached_sub_method(max_time) and
           turn < Parameters::p->nb_iter_local_search and
           best_solution.score_wvcp() != Parameters::p->target) {
        ++turn;
        std::vector<Coloration> best_coloration;
        // int best_evaluation = std::numeric_limits<int>::max();
        auto possible_colors = solution.non_empty_colors();
        possible_colors.push_back(-1);

        int nb_improve = 0;
        int nb_regress = 0;
        int nb_neutral = 0;

        for (int vertex = 0; vertex < Graph::g->nb_vertices; ++vertex) {
            for (const int &color : possible_colors) {
                if (color == solution.color(vertex) or
                    (color != -1 and solution.conflicts_colors(color, vertex) != 0)) {
                    continue;
                }
                const int delta = solution.delta_wvcp_score(vertex, color);
                // const int test_score = solution.score_wvcp() + delta;

                if (delta == 0) {
                    ++nb_neutral;
                } else if (delta > 0) {
                    ++nb_regress;
                } else {
                    ++nb_improve;
                }

                best_coloration.emplace_back(Coloration{vertex, color});
            }
        }
        const float total = static_cast<float>(nb_improve + nb_regress + nb_neutral);
        fmt::print("{},{},{},{},{},{}\n",
                   turn,
                   solution.score_wvcp(),
                   static_cast<float>(nb_improve) / total,
                   static_cast<float>(nb_regress) / total,
                   static_cast<float>(nb_neutral) / total,
                   total);

        const Coloration chosen_one = rd::choice(best_coloration);
        solution.delete_from_color(chosen_one.vertex);
        solution.add_to_color(chosen_one.vertex, chosen_one.color);
        tabu_list[chosen_one.vertex] = turn + solution.nb_non_empty_colors();
        if (solution.score_wvcp() < best_solution.score_wvcp()) {
            best_solution = solution;
            // if (verbose) {
            //     best_time = Parameters::p->elapsed_time(
            //         std::chrono::high_resolution_clock::now());
            //     print_result_ls(best_time, best_solution, turn);
            // }
        } else if (solution.score_wvcp() == best_solution.score_wvcp()) {
            best_solution = solution;
        }
    }
    // if (verbose) {
    //     print_result_ls(best_time, best_solution, turn);
    // }
}
