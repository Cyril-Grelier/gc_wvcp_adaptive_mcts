#include "redls_freeze.hpp"

#include <cassert>

#include "../utils/random_generator.hpp"
#include "../utils/utils.hpp"

void redls_freeze(Solution &best_solution, const bool verbose) {
    auto max_time = std::chrono::high_resolution_clock::now() +
                    std::chrono::seconds(Parameters::p->max_time_local_search);

    int64_t best_time = 0;

    ProxiSolutionRedLS solution(best_solution);
    std::vector<bool> tabu_list(Graph::g->nb_vertices, false);
    long turn = 0;
    while (not Parameters::p->time_limit_reached_sub_method(max_time) and
           turn < Parameters::p->nb_iter_local_search and
           best_solution.score_wvcp() != Parameters::p->target) {
        ++turn;
        if (solution.penalty() == 0) {
            while (improve_conflicts_freeze(solution, false, tabu_list)) {
                assert(solution.check_solution());
            }

            if (solution.score_wvcp() < best_solution.score_wvcp()) {
                best_solution = solution.solution();
                if (verbose) {
                    best_time = Parameters::p->elapsed_time(
                        std::chrono::high_resolution_clock::now());
                    print_result_ls(best_time, best_solution, turn);
                }
                if (solution.score_wvcp() == Parameters::p->target) {
                    if (verbose) {
                        print_result_ls(best_time, best_solution, turn);
                    }
                    return;
                }
            } else if (solution.score_wvcp() == best_solution.score_wvcp()) {
                best_solution = solution.solution();
            }
            std::fill(tabu_list.begin(), tabu_list.end(), false);
            if (not move_heaviest_vertices_freeze(solution)) {
                // not colors available
                return;
            }
            assert(solution.check_solution());
        }

        if (not improve_conflicts_and_score_freeze(
                solution, best_solution.score_wvcp(), tabu_list)) {

            while (improve_conflicts_freeze(solution, true, tabu_list)) {
                assert(solution.check_solution());
            }

            if (not solve_one_conflict_preserve_score_freeze(
                    solution, best_solution.score_wvcp(), tabu_list)) {
                // Increments edge weight
                solution.increment_edge_weights();
                if (not solution.conflict_edges().empty()) {
                    if (not solve_one_conflict_freeze(
                            solution, best_solution.score_wvcp(), tabu_list)) {
                        return;
                    }
                }
            }
            assert(solution.check_solution());
        }
    }
    if (verbose) {
        print_result_ls(best_time, best_solution, turn);
    }
}

bool improve_conflicts_and_score_freeze(ProxiSolutionRedLS &solution,
                                        const int best_local_score,
                                        std::vector<bool> &tabu_list) {
    std::vector<Coloration> best_colorations;
    const int delta_wvcp = std::abs(best_local_score - solution.score_wvcp());
    int best_conflicts = 0;
    for (int vertex = solution.first_free_vertex(); vertex < Graph::g->nb_vertices;
         ++vertex) {
        // for each vertices in conflict
        if (tabu_list[vertex] or not solution.has_conflicts(vertex)) {
            continue;
        }
        for (const auto &color : solution.non_empty_colors()) {
            // for each color different from the color of the vertex
            if (color == solution.color(vertex)) {
                continue;
            }
            const int delta_conflicts = solution.delta_conflicts(vertex, color);
            if (delta_conflicts >= 0 or delta_conflicts > best_conflicts or
                solution.delta_wvcp_score(vertex, color) >= delta_wvcp) {
                continue;
            }
            // pick the move that reduce the most the number of conflicts while
            // improving the score
            if (delta_conflicts < best_conflicts) {
                best_conflicts = delta_conflicts;
                best_colorations.clear();
            }
            best_colorations.emplace_back(Coloration{vertex, color});
        }
    }

    if (best_colorations.empty()) {
        return false;
    }
    // pick one of the best improving move randomly and apply it
    const auto best_move = rd::choice(best_colorations);
    solution.delete_from_color(best_move.vertex);
    solution.add_to_color(best_move.vertex, best_move.color);
    // the vertex become tabu while its neighbors get out of the tabu list
    tabu_list[best_move.vertex] = true;
    for (const auto &neighbor : Graph::g->neighborhood[best_move.vertex]) {
        tabu_list[neighbor] = false;
    }

    return true;
}

bool improve_conflicts_freeze(ProxiSolutionRedLS &solution,
                              const bool with_conf,
                              std::vector<bool> &tabu_list) {
    // find a move that improve the number of conflict and possibly the score
    // if with_conf is true then the vertex must not be in the tabu list
    // if with_conf is true then the chosen vertex will be tabu after the move
    std::vector<Coloration> best_colorations;
    for (int vertex = solution.first_free_vertex(); vertex < Graph::g->nb_vertices;
         ++vertex) {
        if (with_conf and tabu_list[vertex]) {
            continue;
        }
        for (const auto &color : solution.non_empty_colors()) {
            if (color == solution.color(vertex) or
                solution.delta_conflicts(vertex, color) > 0 or
                solution.delta_wvcp_score(vertex, color) >= 0) {
                continue;
            }
            best_colorations.emplace_back(Coloration{vertex, color});
        }
    }

    if (best_colorations.empty()) {
        return false;
    }

    const auto chosen_one = rd::choice(best_colorations);
    solution.delete_from_color(chosen_one.vertex);
    solution.add_to_color(chosen_one.vertex, chosen_one.color);

    if (with_conf) {
        tabu_list[chosen_one.vertex] = true;
    }

    return true;
}

bool solve_one_conflict_preserve_score_freeze(ProxiSolutionRedLS &solution,
                                              const int best_local_score,
                                              std::vector<bool> &tabu_list) {
    // select all non tabu vertices in conflict that keep the wvcp
    // score lower than the best found score
    // the vertex will be tabu after the move
    const int delta_wvcp = best_local_score - solution.score_wvcp();
    std::vector<int> vertices;
    for (int vertex = solution.first_free_vertex(); vertex < Graph::g->nb_vertices;
         ++vertex) {
        if (tabu_list[vertex] == false and solution.has_conflicts(vertex) and
            solution.delta_wvcp_score(vertex, -1) < delta_wvcp) {
            vertices.emplace_back(vertex);
        }
    }

    if (vertices.empty()) {
        return false;
    }

    const int vertex = rd::choice(vertices);
    solution.delete_from_color(vertex);
    solution.add_to_color(vertex, -1);
    tabu_list[vertex] = true;

    return true;
}

bool move_heaviest_vertices_freeze(ProxiSolutionRedLS &solution) {
    // look for the best set of heaviest vertices in each colors
    // to relocate them in an other color without increasing too
    // much the number of conflicts
    float best_ratio = 0;
    int best_color = -1;
    std::set<int> best_heaviest_vertices;
    const int first_free_vertex = solution.first_free_vertex();
    for (const auto &color1 : solution.non_empty_colors()) {
        const int max_weight1 = solution.max_weight(color1);
        int second_max = 0;
        std::set<int> heaviest_vertices;
        for (const auto &vertex : solution.colors_vertices(color1)) {
            const int vertex_weight = Graph::g->weights[vertex];
            if (vertex_weight == max_weight1) {
                heaviest_vertices.insert(vertex);
            } else if (vertex_weight > second_max) {
                second_max = vertex_weight;
            }
        }
        const int first_vertex = *heaviest_vertices.begin();
        if (first_vertex < first_free_vertex) {
            continue;
        }
        const int delta_move = max_weight1 - second_max;
        for (const auto &color2 : solution.non_empty_colors()) {
            if (color1 == color2) {
                continue;
            }
            int delta_conflict = 0;
            for (const auto &vertex : heaviest_vertices) {
                delta_conflict += solution.conflicts_colors(color2, vertex);
            }
            const int max_weight2 = solution.max_weight(color2);
            const int delta_wvcp = max_weight1 > max_weight2
                                       ? delta_move - max_weight1 + max_weight2
                                       : delta_move;

            float ratio = delta_conflict != 0 ? static_cast<float>(delta_wvcp) /
                                                    static_cast<float>(delta_conflict)
                                              : static_cast<float>(delta_wvcp) * 2;
            if (ratio > best_ratio or best_heaviest_vertices.empty()) {
                best_ratio = ratio;
                best_heaviest_vertices = heaviest_vertices;
                best_color = color2;
            }
        }
    }
    if (best_heaviest_vertices.empty()) {
        return false;
    }

    for (const auto &vertex : best_heaviest_vertices) {
        solution.delete_from_color(vertex);
        solution.add_to_color(vertex, best_color);
        // Probably not tabu
        // tabu_list[vertex] = 1;
    }
    return true;
}

bool solve_one_conflict_freeze(ProxiSolutionRedLS &solution,
                               const int best_local_score,
                               std::vector<bool> &tabu_list) {
    // Pick a random pair of vertices in conflicts and find the best move
    // possibly improving the score and the number of vertices in conflicts
    // otherwise a random move in a random color
    const int delta_wvcp = best_local_score - solution.score_wvcp();
    Coloration best_coloration{-1, -1};
    const auto &[v1_, v2_] = rd::choice(solution.conflict_edges());
    int v1 = v1_;
    int v2 = v2_;
    bool changed = false;
    const int first_free_vertex = solution.first_free_vertex();
    if (v1 < first_free_vertex) {
        v1 = v2;
        changed = true;
    }
    if (v2 < first_free_vertex) {
        if (changed) {
            // this case should not happened as it should not be possible
            // for 2 freezed vertices to be in conflict
            fmt::print(
                stderr,
                "RedLS : selectionRule2(solve_one_conflict) should not be possible !");
            return false;
        }
        v2 = v1;
    }
    int best_score_conflicts = 0;
    for (const int &vertex : {v1, v2}) {
        for (const auto &color : solution.non_empty_colors()) {
            if (color == solution.color(vertex) or
                solution.delta_wvcp_score(vertex, color) >= delta_wvcp) {
                continue;
            }
            int delta_conflicts_score = solution.delta_conflicts(vertex, color);
            if (delta_conflicts_score < best_score_conflicts or
                best_coloration.vertex == -1) {
                best_score_conflicts = delta_conflicts_score;
                best_coloration = Coloration{vertex, color};
            }
        }
    }

    if (best_coloration.vertex == -1) {
        const std::vector<int> edges = {v1, v2};
        const int vertex = rd::choice(edges);
        std::vector<int> possible_colors{-1};
        for (const auto &color : solution.non_empty_colors()) {
            if (color != solution.color(vertex)) {
                possible_colors.emplace_back(color);
            }
        }
        const int color = rd::choice(possible_colors);

        best_coloration = Coloration{vertex, color};
    }

    solution.delete_from_color(best_coloration.vertex);
    solution.add_to_color(best_coloration.vertex, best_coloration.color);
    tabu_list[best_coloration.vertex] = true;
    return true;
}
