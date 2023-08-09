#include "ilsts.hpp"

#include <algorithm>
#include <cassert>

#include "../utils/random_generator.hpp"
#include "../utils/utils.hpp"

void ilsts(Solution &best_solution, const bool verbose) {

    auto max_time = std::chrono::high_resolution_clock::now() +
                    std::chrono::seconds(Parameters::p->max_time_local_search);

    int64_t best_time = 0;

    ProxiSolutionILSTS working_solution(best_solution);
    std::vector<long> tabu(Graph::g->nb_vertices, 0);

    long no_improve = 0; // number of iterations without improvement
    long turn = 0;
    int force = 1; // perturbation strength
    // vertices to shuffle to explore randomly
    std::vector<int> vertices;
    vertices.resize(Graph::g->nb_vertices);
    std::iota(vertices.begin(), vertices.end(), 0);

    while (turn < Parameters::p->nb_iter_local_search and
           not Parameters::p->time_limit_reached_sub_method(max_time) and
           best_solution.score_wvcp() != Parameters::p->target) {
        ++turn;
        ProxiSolutionILSTS next_s(working_solution);

        next_s.unassigned_random_heavy_vertices(force);

        long iter = 0;
        while (next_s.has_unassigned_vertices() and iter < Graph::g->nb_vertices * 10 and
               not Parameters::p->time_limit_reached_sub_method(max_time)) {
            ++iter;

            if (next_s.has_unassigned_vertices() and M_1_2_3(next_s, iter, tabu)) {
                assert(next_s.check_solution());
                continue;
            }

            std::shuffle(vertices.begin(), vertices.end(), rd::generator);
            if (next_s.has_unassigned_vertices() and M_4(next_s, iter, vertices, tabu)) {
                assert(next_s.check_solution());
                continue;
            }

            if (next_s.has_unassigned_vertices() and M_5(next_s, iter, vertices, tabu)) {
                assert(next_s.check_solution());
                continue;
            }

            if (next_s.has_unassigned_vertices() and M_6(next_s, iter, tabu)) {
                assert(next_s.check_solution());
                continue;
            }
            break;
        }
        if (next_s.get_score() < working_solution.get_score()) {
            no_improve = 1;
            working_solution = next_s;
            force = 1;
        } else if (no_improve <= Graph::g->nb_vertices) {
            ++no_improve;
            if (force == 3) {
                force = 1;
            } else {
                ++force;
            }
        } else {
            working_solution.perturb_vertices(1);
            no_improve = 1;
        }

        if ((not working_solution.has_unassigned_vertices()) and
            (best_solution.score_wvcp() > working_solution.score_wvcp())) {
            best_solution = working_solution.solution();
            if (verbose) {
                best_time = Parameters::p->elapsed_time(
                    std::chrono::high_resolution_clock::now());
                print_result_ls(best_time, best_solution, turn);
            }
        } else if ((not working_solution.has_unassigned_vertices()) and
                   (best_solution.score_wvcp() == working_solution.score_wvcp())) {
            best_solution = working_solution.solution();
        }
    }
    if (verbose) {
        print_result_ls(best_time, best_solution, turn);
    }
}

bool M_1_2_3(ProxiSolutionILSTS &solution, const long iter, std::vector<long> &tabu) {
    const int delta = solution.unassigned_score() - solution.score_wvcp();
    // M1 : moves a vertex without increasing the score

    // M2 : moves a vertex without increasing the score and moves its neighbors to other
    // colors (perfect grenade)

    // M3 : moves a vertex without increasing the score and moves its
    // neighbors to other colors except for one that become uncolored (grenade one lost)

    // list(vertex, color) for M3 grenade
    std::vector<std::tuple<int, int>> grenade_one_lost;
    auto non_empty_colors = solution.non_empty_colors();
    shuffle(non_empty_colors.begin(), non_empty_colors.end(), rd::generator);
    for (const auto &vertex : solution.unassigned()) {

        // M1 : move vertex to a color with no neighbors that doesn't increase the score
        const int vertex_weight = Graph::g->weights[vertex];
        for (const auto &color : non_empty_colors) {
            if (solution.conflicts_colors(color, vertex) == 0 and
                delta > std::max(0, vertex_weight - solution.max_weight(color))) {
                solution.add_to_color(vertex, color);
                solution.remove_unassigned_vertex(vertex);
                return true;
            }
        }

        // M2 : move vertex to a color with neighbors that doesn't increase the score
        //
        // costs counts the number of neighbors in the tabu list
        std::vector<int> costs(solution.nb_colors(), 0);
        // relocated counts the number of neighbors that must be relocated for each color
        std::vector<int> relocated(solution.nb_colors(), 0);
        for (const auto &neighbor : Graph::g->neighborhood[vertex]) {
            // if neighbor is unassigned, we don't care
            int neighbor_color = solution.color(neighbor);
            if (neighbor_color == -1) {
                continue;
            }

            // if neighbor is assigned to a color that increases the score, we don't care
            if (delta <=
                std::max(0, vertex_weight - solution.max_weight(neighbor_color))) {
                continue;
            }

            // if the number of free colors for the neighbor is > 0, we increase the
            // number of relocated for the color
            if (solution.nb_free_colors(neighbor) > 0) {
                ++relocated[neighbor_color];
            } else if (tabu[neighbor] < iter) {
                ++relocated[neighbor_color];
                ++costs[neighbor_color];
            }

            // if not all neighbors in the color have been explored, we continue
            if (relocated[neighbor_color] !=
                solution.conflicts_colors(neighbor_color, vertex)) {
                continue;
            }

            // if all neighbors can be relocated, we apply the move
            if (costs[neighbor_color] == 0) {
                // we apply the grenade move on the vertex and neighbors
                std::vector<int> unassigned;
                for (int y : Graph::g->neighborhood[vertex]) {
                    if (solution.color(y) == neighbor_color) {
                        assert(solution.nb_free_colors(y) > 0);
                        solution.delete_from_color(y);
                        unassigned.push_back(y);
                    }
                }
                if (solution.is_color_empty(neighbor_color)) {
                    neighbor_color = -1;
                }
                solution.add_to_color(vertex, neighbor_color);

                solution.random_assignment_constrained(unassigned);
                assert(unassigned.empty());
                solution.remove_unassigned_vertex(vertex);
                return true;
            }
            // if there is only one neighbor that become uncolored, keep the move for M3
            if (costs[neighbor_color] == 1) {
                grenade_one_lost.emplace_back(vertex, neighbor_color);
            }
        }
    }
    // M3 : move vertex to a color with neighbors that doesn't increase the score and only
    // one neighbor that become uncolored
    return M_3(solution, iter, grenade_one_lost, tabu);
}

bool M_3(ProxiSolutionILSTS &solution,
         const long iter,
         const std::vector<std::tuple<int, int>> &grenade_one_lost,
         std::vector<long> &tabu) {
    // M3 : move vertex to a color with neighbors that doesn't increase the score and only
    // one neighbor that become uncolored
    if (grenade_one_lost.empty()) {
        return false;
    }
    const auto [vertex, min_color] = rd::choice(grenade_one_lost);
    std::vector<int> unassigned;
    for (const auto &y : Graph::g->neighborhood[vertex]) {
        if (solution.color(y) == min_color) {
            if (solution.nb_free_colors(y) > 0) {
                unassigned.push_back(y);
            } else {
                solution.add_unassigned_vertex(y);
            }
            solution.delete_from_color(y);
        }
    }

    solution.add_to_color(vertex, solution.is_color_empty(min_color) ? -1 : min_color);
    tabu[vertex] = iter + static_cast<long>(solution.non_empty_colors().size());
    solution.random_assignment_constrained(unassigned);
    assert(unassigned.empty());
    solution.remove_unassigned_vertex(vertex);
    return true;
}

bool M_4(ProxiSolutionILSTS &solution,
         const long iter,
         const std::vector<int> &vertices,
         std::vector<long> &tabu) {
    // M4 : for each colored vertex not tabu with free colors, move it to an other color
    // move at most |non_empty_colors| vertices
    const long max_counter = static_cast<long>(solution.non_empty_colors().size());
    int counter = 0;
    for (const auto &vertex : vertices) {
        if (solution.nb_free_colors(vertex) > 0 and tabu[vertex] < iter and
            solution.color(vertex) != -1) {
            tabu[vertex] = iter + static_cast<long>(solution.non_empty_colors().size());
            solution.random_assignment_constrained(vertex);
            ++counter;
            if (counter == max_counter) {
                return true;
            }
        }
    }
    return (counter > 0);
}

bool M_5(ProxiSolutionILSTS &solution,
         const long iter,
         const std::vector<int> &vertices,
         std::vector<long> &tabu) {
    // M5 : for each vertex try to relocate its neighbors to change the color of the
    // vertex without increasing the score
    const int delta = solution.unassigned_score() - solution.score_wvcp();

    for (const auto &vertex : vertices) {
        const bool has_free_colors = solution.nb_free_colors(vertex) != 0;
        const bool is_not_tabu = tabu[vertex] >= iter;
        const bool is_not_colored = solution.color(vertex) == -1;

        if (has_free_colors or is_not_tabu or is_not_colored) {
            continue;
        }
        std::vector<int> relocated(solution.nb_colors(), 0);
        for (const auto &neighbor : Graph::g->neighborhood[vertex]) {
            int c_neighbor = solution.color(neighbor);
            if (c_neighbor == -1)
                continue;
            if (delta >
                std::max(0,
                         Graph::g->weights[vertex] - solution.max_weight(c_neighbor))) {
                if (solution.nb_free_colors(neighbor) > 0) {
                    ++relocated[c_neighbor];
                }
            }
            if (relocated[c_neighbor] == solution.conflicts_colors(c_neighbor, vertex)) {
                std::vector<int> unassigned;
                for (const auto &y : Graph::g->neighborhood[vertex]) {
                    if (solution.color(y) == c_neighbor and
                        solution.nb_free_colors(y) > 0) {
                        solution.delete_from_color(y);
                        unassigned.push_back(y);
                    }
                }
                solution.delete_from_color(vertex);
                tabu[vertex] =
                    iter + static_cast<long>(solution.non_empty_colors().size());
                solution.add_to_color(
                    vertex, solution.is_color_empty(c_neighbor) ? -1 : c_neighbor);

                solution.random_assignment_constrained(unassigned);
                return true;
            }
        }
    }
    return false;
}

bool M_6(ProxiSolutionILSTS &solution, const long iter, std::vector<long> &tabu) {
    int min_cost = Graph::g->nb_vertices;
    int min_cost_c = -1;
    const int delta = solution.unassigned_score() - solution.score_wvcp();
    // M6 : pick a random uncolored vertex and try to relocate its neighbors
    const int vertex = rd::choice(solution.unassigned());

    std::vector<int> relocated(solution.nb_colors(), 0);
    std::vector<int> costs(solution.nb_colors(), 0);

    for (const auto &neighbor : Graph::g->neighborhood[vertex]) {
        const int c_neighbor = solution.color(neighbor);
        if (c_neighbor == -1)
            continue;
        if (delta >
            std::max(0, Graph::g->weights[vertex] - solution.max_weight(c_neighbor))) {
            if (solution.nb_free_colors(neighbor) > 0) {
                ++relocated[c_neighbor];
            } else {
                ++relocated[c_neighbor];
                ++costs[c_neighbor];
            }

            if (relocated[c_neighbor] == solution.conflicts_colors(c_neighbor, vertex) and
                min_cost > costs[c_neighbor]) {
                min_cost_c = c_neighbor;
                min_cost = costs[c_neighbor];
            }
        }
    }

    if (min_cost_c == -1) {
        return false;
    }
    std::vector<int> unassigned;
    std::fill(tabu.begin(), tabu.end(), 0);
    for (const auto &y : Graph::g->neighborhood[vertex]) {
        if (solution.color(y) == min_cost_c) {
            if (solution.nb_free_colors(y) > 0) {
                unassigned.push_back(y);
            } else {
                solution.add_unassigned_vertex(y);
            }
            solution.delete_from_color(y);
        }
    }
    tabu[vertex] = iter + static_cast<long>(solution.non_empty_colors().size());
    if (solution.is_color_empty(min_cost_c))
        min_cost_c = -1;
    solution.add_to_color(vertex, min_cost_c);

    solution.random_assignment_constrained(unassigned);
    assert(unassigned.empty());
    solution.remove_unassigned_vertex(vertex);
    return true;
}
