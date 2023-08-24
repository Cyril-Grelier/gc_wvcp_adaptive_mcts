#include "greedy.hpp"

#include <cassert>

#include "../utils/random_generator.hpp"
#include "../utils/utils.hpp"

void total_random(Solution &solution) {
    // shuffle the vertices
    std::vector<int> vertices(Graph::g->nb_vertices);
    std::iota(vertices.begin(), vertices.end(), 0);
    std::shuffle(vertices.begin(), vertices.end(), rd::generator);
    for (const auto vertex : vertices) {
        auto possible_colors = solution.available_colors(vertex);
        solution.add_to_color(vertex, rd::choice(possible_colors));
    }
}

void greedy_random(Solution &solution) {
    for (int vertex = solution.first_free_vertex(); vertex < Graph::g->nb_vertices;
         ++vertex) {
        auto possible_colors = solution.available_colors(vertex);
        // add -1 to have the possibility to open a new color even if not needed
        possible_colors.emplace_back(-1);
        solution.add_to_color(vertex, rd::choice(possible_colors));
    }
}

void greedy_constrained(Solution &solution) {
    for (int vertex = solution.first_free_vertex(); vertex < Graph::g->nb_vertices;
         ++vertex) {
        auto possible_colors = solution.available_colors(vertex);
        solution.add_to_color(vertex, rd::choice(possible_colors));
    }
}

void greedy_deterministic(Solution &solution) {
    for (int vertex = solution.first_free_vertex(); vertex < Graph::g->nb_vertices;
         ++vertex) {
        solution.add_to_color(vertex, solution.first_available_color(vertex));
    }
}

void greedy_worst(Solution &solution) {
    for (int vertex = solution.first_free_vertex(); vertex < Graph::g->nb_vertices;
         ++vertex) {
        solution.add_to_color(vertex, -1);
    }
}

// Struct used in conjunction with the DSatur priority queue
struct satItem {
    int sat;
    int weight;
    int deg;
    int vertex;
};

struct maxSat {
    bool operator()(const satItem &lhs, const satItem &rhs) const {
        if (lhs.weight > rhs.weight)
            return true;
        if (lhs.weight < rhs.weight)
            return false;
        // Compares two satItems sat deg, then degree, then vertex label
        if (lhs.sat > rhs.sat)
            return true;
        if (lhs.sat < rhs.sat)
            return false;
        // if we are we know that lhs.sat == rhs.sat

        if (lhs.deg > rhs.deg)
            return true;
        if (lhs.deg < rhs.deg)
            return false;
        // if we are here we know that lhs.sat == rhs.sat and lhs.deg == rhs.deg.
        // Our choice can be arbitrary
        // edit : vertices are sorted by degree so we keep this order here
        if (lhs.vertex < rhs.vertex)
            return true;
        return false;
    }
};

void greedy_DSatur(Solution &solution) {
    std::vector<int> degrees(Graph::g->degrees);
    std::vector<std::set<int>> adjacent_colors(Graph::g->nb_vertices, std::set<int>());
    std::set<satItem, maxSat> Q;

    // if the solution already has vertices colored, we need to update the data structures
    if (solution.first_free_vertex() != Graph::g->nb_vertices) {
        for (int vertex = 0; vertex < Graph::g->nb_vertices; ++vertex) {
            const auto color = solution.color(vertex);
            if (color != -1) {
                for (const int neighbor : Graph::g->neighborhood[vertex]) {
                    if (solution.color(neighbor) == -1) {
                        adjacent_colors[neighbor].insert(color);
                        --degrees[neighbor];
                    }
                }
            }
        }
    }

    // Initialise the the data structures.
    // These are a priority queue,
    // a set of colors adjacent to each uncolored vertex (initially empty) and the degree
    // d(v) of each uncolored vertex in the graph induced by uncolored vertices
    for (int vertex = 0; vertex < Graph::g->nb_vertices; ++vertex) {
        if (solution.color(vertex) == -1) {
            Q.emplace(satItem{static_cast<int>(adjacent_colors[vertex].size()),
                              Graph::g->weights[vertex],
                              degrees[vertex],
                              vertex});
        }
    }

    while (!Q.empty()) {
        // Get the vertex u with highest saturation degree, breaking ties with d.
        auto maxPtr = Q.begin();
        // fmt::print("sat : {}, weight : {}, deg : {}, vertex : {}\n",
        //            (*maxPtr).sat,
        //            (*maxPtr).weight,
        //            (*maxPtr).deg,
        //            (*maxPtr).vertex);
        const int vertex = (*maxPtr).vertex;
        // Remove it from the priority queue
        Q.erase(maxPtr);
        // and color it
        int color = -1;
        for (int color_ = 0; color_ < solution.nb_colors(); ++color_) {
            if (solution.conflicts_colors(color_, vertex) == 0) {
                color = color_;
                break;
            }
        }

        solution.add_to_color(vertex, color);

        // Update the saturation degrees and d-value of all uncolored neighbors; hence
        // modify their corresponding elements in the priority queue
        for (const int neighbor : Graph::g->neighborhood[vertex]) {
            if (solution.color(neighbor) == -1) {
                Q.erase({static_cast<int>(adjacent_colors[neighbor].size()),
                         Graph::g->weights[neighbor],
                         degrees[neighbor],
                         neighbor});
                adjacent_colors[neighbor].insert(color);
                --degrees[neighbor];
                Q.emplace(satItem{static_cast<int>(adjacent_colors[neighbor].size()),
                                  Graph::g->weights[neighbor],
                                  degrees[neighbor],
                                  neighbor});
            }
        }
    }
}

void update_neighborhood(const Solution &solution,
                         std::vector<int> &legal_uncolored,
                         std::vector<int> &illegal_uncolored,
                         std::vector<int> &nb_uncolored_neighbors,
                         std::vector<int> &nb_illegal_neighbors,
                         int current_vertex) {

    erase_sorted(legal_uncolored, current_vertex);
    // legal_uncolored.erase(current_vertex);

    // each uncolored neighbor of the current vertex goes in the illegal set
    // the neighbor and its neighbors are impacted by the move
    std::vector<int> impacted_vertices;
    for (const int neighbor : Graph::g->neighborhood[current_vertex]) {
        if (solution.color(neighbor) == -1) {
            erase_sorted(legal_uncolored, neighbor);
            // legal_uncolored.erase(neighbor);
            // illegal_uncolored.insert(neighbor);
            if (not contains(illegal_uncolored, neighbor))
                insert_sorted(illegal_uncolored, neighbor);
            // impacted_vertices.insert(neighbor);
            if (not contains(impacted_vertices, neighbor))
                insert_sorted(impacted_vertices, neighbor);
            for (int w : Graph::g->neighborhood[neighbor]) {
                if (solution.color(w) == -1) {
                    if (not contains(impacted_vertices, w))
                        insert_sorted(impacted_vertices, w);
                    // impacted_vertices.insert(w);
                }
            }
        }
    }

    // Recalculate the nb of neighbors legal and illegal for each
    // impacted vertex
    for (const int vertex : impacted_vertices) {
        nb_uncolored_neighbors[vertex] = 0;
        nb_illegal_neighbors[vertex] = 0;
        for (int neighbor : Graph::g->neighborhood[vertex]) {
            if (solution.color(neighbor) == -1) {
                // if (legal_uncolored.count(neighbor) == 1)
                if (contains(legal_uncolored, neighbor))
                    nb_uncolored_neighbors[vertex]++;
                // else if (illegal_uncolored.count(neighbor) == 1)
                else if (contains(illegal_uncolored, neighbor))
                    nb_illegal_neighbors[vertex]++;
            }
        }
    }
}

void greedy_RLF(Solution &solution) {
    // vertices to color
    std::vector<int> legal_uncolored;
    for (int vertex = 0; vertex < Graph::g->nb_vertices; ++vertex) {
        if (solution.color(vertex) == -1) {
            insert_sorted(legal_uncolored, vertex);
        }
    }

    // vertices that have neighbors in the current color
    std::vector<int> illegal_uncolored;

    int color = -1;

    while (not legal_uncolored.empty()) {
        ++color;

        // nb neighbors uncolored
        std::vector<int> nb_uncolored_neighbors(Graph::g->nb_vertices, 0);
        // nb neighbors uncolored that won't be in the current color
        std::vector<int> nb_illegal_neighbors(Graph::g->nb_vertices, 0);

        for (int vertex = 0; vertex < Graph::g->nb_vertices; ++vertex) {
            if (solution.color(vertex) == color) {
                for (const auto neighbor : Graph::g->neighborhood[vertex]) {
                    if (solution.color(neighbor) == -1) {
                        if (contains(legal_uncolored, neighbor)) {
                            erase_sorted(legal_uncolored, neighbor);
                        }
                        if (not contains(illegal_uncolored, neighbor)) {
                            insert_sorted(illegal_uncolored, neighbor);
                            for (const auto neighbor2 :
                                 Graph::g->neighborhood[neighbor]) {
                                ++nb_illegal_neighbors[neighbor2];
                            }
                        }
                    }
                }
            } else if (solution.color(vertex) == -1) {
                for (int neighbor : Graph::g->neighborhood[vertex]) {
                    if (contains(legal_uncolored, neighbor)) {
                        nb_uncolored_neighbors[vertex]++;
                    }
                }
            }
        }

        if (solution.nb_colors() <= color) {
            // update the nb of uncolored neighbors and select
            // the first vertex in the color, the one that has
            // the highest number of neighbors uncolored
            int first_vertex = -1;
            int nb_max_neighbors_uncolored = -1;
            for (const int vertex : legal_uncolored) {
                if (nb_uncolored_neighbors[vertex] > nb_max_neighbors_uncolored) {
                    nb_max_neighbors_uncolored = nb_uncolored_neighbors[vertex];
                    first_vertex = vertex;
                }
            }
            const auto used_color = solution.add_to_color(first_vertex, -1);
            (void)used_color;
            assert(used_color == color);

            update_neighborhood(solution,
                                legal_uncolored,
                                illegal_uncolored,
                                nb_uncolored_neighbors,
                                nb_illegal_neighbors,
                                first_vertex);
        }

        while (not legal_uncolored.empty()) {
            // select the next vertex, the one that has that has the highest number of
            // neighbors uncolored and not authorized in the current color.
            // If equality, pick the one that has the lowest number of neighbors uncolored
            // and authorized in the current color.

            int nb_max_neighbors_illegal = -1;
            int weight_next = -1;
            int nb_min_neighbors_legal = Graph::g->nb_vertices;
            int next_vertex = -1;
            for (int vertex : legal_uncolored) {
                if ((nb_illegal_neighbors[vertex] > nb_max_neighbors_illegal and
                     Graph::g->weights[vertex] >= weight_next) or
                    (nb_illegal_neighbors[vertex] == nb_max_neighbors_illegal and
                     nb_uncolored_neighbors[vertex] < nb_min_neighbors_legal and
                     Graph::g->weights[vertex] == weight_next)) {
                    nb_max_neighbors_illegal = nb_illegal_neighbors[vertex];
                    nb_min_neighbors_legal = nb_uncolored_neighbors[vertex];
                    next_vertex = vertex;
                    weight_next = Graph::g->weights[vertex];
                }
            }
            solution.add_to_color(next_vertex, color);
            update_neighborhood(solution,
                                legal_uncolored,
                                illegal_uncolored,
                                nb_uncolored_neighbors,
                                nb_illegal_neighbors,
                                next_vertex);
        }
        legal_uncolored = illegal_uncolored;
        illegal_uncolored.clear();
    }
}

init_ptr get_initialization_fct(const std::string &initialization) {
    if (initialization == "total_random")
        return total_random;
    if (initialization == "random")
        return greedy_random;
    if (initialization == "constrained")
        return greedy_constrained;
    if (initialization == "deterministic")
        return greedy_deterministic;
    if (initialization == "worst")
        return greedy_worst;
    if (initialization == "rlf")
        return greedy_RLF;
    if (initialization == "dsatur")
        return greedy_DSatur;
    fmt::print(stderr,
               "Unknown initialization, please select : "
               "total_random, random, constrained, deterministic, worst, rlf, dsatur\n");
    exit(1);
}
