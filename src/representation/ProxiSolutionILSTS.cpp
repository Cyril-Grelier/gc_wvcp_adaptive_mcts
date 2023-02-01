#include "ProxiSolutionILSTS.hpp"

#include <cassert>

#include "../utils/random_generator.hpp"

ProxiSolutionILSTS::ProxiSolutionILSTS(Solution solution)
    : _solution(solution),
      _nb_free_colors(Graph::g->nb_vertices, 0),
      _unassigned_score(solution.score_wvcp()) {
    for (int vertex = 0; vertex < Graph::g->nb_vertices; ++vertex) {
        for (int color = 0; color < _solution.nb_colors(); ++color) {
            if (_solution.conflicts_colors(color, vertex) == 0 and
                color != _solution.color(vertex) and
                not _solution.is_color_empty(color) and
                Graph::g->weights[vertex] <= _solution.max_weight(color)) {
                ++_nb_free_colors[vertex];
            }
        }
    }
}

bool ProxiSolutionILSTS::check_solution() const {
    for (const auto &v : _unassigned) {
        (void)v;
        assert(_solution.color(v) == -1);
    }
    int unassigned = 0;
    for (int vertex = 0; vertex < Graph::g->nb_vertices; ++vertex) {
        if (_solution.color(vertex) == -1) {
            ++unassigned;
        }
        int free_color = 0;
        for (int color = 0; color < _solution.nb_colors(); ++color) {
            if (_solution.conflicts_colors(color, vertex) == 0 and
                color != _solution.color(vertex) and
                not _solution.is_color_empty(color) and
                Graph::g->weights[vertex] <= _solution.max_weight(color)) {
                ++free_color;
            }
        }
        assert(_nb_free_colors[vertex] == free_color);
    }
    assert(unassigned == static_cast<int>(_unassigned.size()));

    return _solution.check_solution();
}

int ProxiSolutionILSTS::add_to_color(const int vertex, const int color_proposed) {

    const int old_max_weight = _solution.max_weight(color_proposed);

    const int color = _solution.add_to_color(vertex, color_proposed);

    // update free colors for neighbors
    for (const auto &neighbor : Graph::g->neighborhood[vertex]) {
        if (_solution.conflicts_colors(color, neighbor) == 1 and
            Graph::g->weights[neighbor] <= old_max_weight) {
            _nb_free_colors[neighbor]--;
        }
    }

    // update nb free colors
    // if the vertex increase the class weight
    if (Graph::g->weights[vertex] > old_max_weight) {
        // for all vertices outside the color -> 9s -> 165
        for (int vertex_out = 0; vertex_out < Graph::g->nb_vertices; ++vertex_out) {
            if (_solution.color(vertex_out) == color) {
                continue;
            }
            if (Graph::g->weights[vertex_out] > old_max_weight and
                Graph::g->weights[vertex_out] <= Graph::g->weights[vertex] and
                _solution.conflicts_colors(color, vertex_out) == 0) {
                // update its number of free colors
                ++_nb_free_colors[vertex_out];
            }
        }
    } else {
        // the vertex lost a free color
        --_nb_free_colors[vertex];
    }
    return color;
}

int ProxiSolutionILSTS::delete_from_color(const int vertex) {

    const int old_weight = max_weight(_solution.color(vertex));
    const int color = _solution.delete_from_color(vertex);

    // update free color for neighbors
    for (const int neighbor : Graph::g->neighborhood[vertex]) {
        if (_solution.conflicts_colors(color, neighbor) == 0 and
            Graph::g->weights[neighbor] <= old_weight) {
            ++_nb_free_colors[neighbor];
        }
    }

    const int vertex_weight = Graph::g->weights[vertex];

    const int max_weight_color = max_weight(color);

    // update free colors
    if (vertex_weight == old_weight) {
        for (int vertex_out = 0; vertex_out < Graph::g->nb_vertices; ++vertex_out) {
            if (_solution.color(vertex_out) == color) {
                continue;
            }

            if (Graph::g->weights[vertex_out] <= old_weight and
                Graph::g->weights[vertex_out] > max_weight_color and
                _solution.conflicts_colors(color, vertex_out) == 0 and
                vertex != vertex_out) {
                --_nb_free_colors[vertex_out];
            }
        }
    }

    if (old_weight == max_weight_color) {
        ++_nb_free_colors[vertex];
    }
    return color;
}

[[nodiscard]] int ProxiSolutionILSTS::unassigned_score() const {
    return _unassigned_score;
}

void ProxiSolutionILSTS::unassigned_random_heavy_vertices(const int force) {
    std::uniform_int_distribution<int> distribution(0, _solution.nb_colors() - 1);
    std::vector<int> unassigned;
    _unassigned_score = _solution.score_wvcp();
    for (int i = 0; i < force; ++i) {
        const int color = rd::choice(_solution.non_empty_colors());
        const int old_max_weight = max_weight(color);

        std::vector<int> to_unassign;
        for (const auto &vertex : _solution.colors_vertices(color)) {
            if (Graph::g->weights[vertex] == old_max_weight) {
                to_unassign.push_back(vertex);
            }
        }
        for (const auto &vertex : to_unassign) {
            delete_from_color(vertex);
            unassigned.push_back(vertex);
        }
    }

    std::shuffle(unassigned.begin(), unassigned.end(), rd::generator);
    for (const int &vertex : unassigned) {
        if (not random_assignment_constrained(vertex)) {
            _unassigned.emplace_back(vertex);
        }
    }
}

void ProxiSolutionILSTS::perturb_vertices(const int force) {
    assert(_unassigned.empty());
    std::uniform_int_distribution<int> distribution_v(0, Graph::g->nb_vertices - 1);
    std::uniform_int_distribution<int> distribution_c(0, _solution.nb_colors() - 1);

    for (int i = 0; i < force; ++i) {
        int vertex = 0;
        int color = _solution.color(vertex);
        while (_solution.color(vertex) == color) {
            vertex = distribution_v(rd::generator);
            color = rd::choice(_solution.non_empty_colors());
        }

        delete_from_color(vertex);

        std::vector<int> unassigned;
        for (const int neighbor : Graph::g->neighborhood[vertex]) {
            if (_solution.color(neighbor) == color) {
                unassigned.push_back(neighbor);
                delete_from_color(neighbor);
            }
        }

        if (_solution.is_color_empty(color)) {
            color = -1;
        }

        add_to_color(vertex, color);
        std::random_shuffle(unassigned.begin(), unassigned.end());

        std::vector<int> to_random;
        // assigned without increasing score
        for (const int &v : unassigned) {
            if (not random_assignment_constrained(v)) {
                to_random.emplace_back(v);
            }
        }
        // assigned with increasing score
        for (const int &v : to_random) {
            add_to_color(v, rd::choice(_solution.available_colors(v)));
        }
    }
}

bool ProxiSolutionILSTS::random_assignment_constrained(const int vertex) {
    const auto available_colors = _solution.available_colors(vertex);
    if (available_colors[0] == -1) {
        return false;
    }
    const int vertex_weight = Graph::g->weights[vertex];
    const int vertex_color = _solution.color(vertex);
    std::vector<int> possible_color;
    for (const auto &color : available_colors) {
        if (color != vertex_color and vertex_weight <= _solution.max_weight(color)) {
            possible_color.emplace_back(color);
        }
    }

    if (vertex_color != -1) {
        delete_from_color(vertex);
    }

    if (possible_color.empty()) {
        return false;
    }

    add_to_color(vertex, rd::choice(possible_color));

    return true;
}

void ProxiSolutionILSTS::random_assignment_constrained(std::vector<int> &vertices) {
    std::vector<int> unassigned;

    for (const auto &vertex : vertices) {
        if (!random_assignment_constrained(vertex)) {
            unassigned.push_back(vertex);
        }
    }
    vertices = unassigned;
}

[[nodiscard]] int ProxiSolutionILSTS::get_score() const {
    return _unassigned.empty() ? _solution.score_wvcp() : _unassigned_score;
}

[[nodiscard]] const std::vector<int> &ProxiSolutionILSTS::unassigned() const {
    return _unassigned;
}

[[nodiscard]] int ProxiSolutionILSTS::nb_free_colors(const int &vertex) const {
    return _nb_free_colors[vertex];
}

[[nodiscard]] bool ProxiSolutionILSTS::has_unassigned_vertices() const {
    return not _unassigned.empty();
}

void ProxiSolutionILSTS::add_unassigned_vertex(const int &vertex) {
    _unassigned.push_back(vertex);
}

void ProxiSolutionILSTS::remove_unassigned_vertex(const int &vertex) {
    _unassigned.erase(std::remove(_unassigned.begin(), _unassigned.end(), vertex));
}

[[nodiscard]] Solution ProxiSolutionILSTS::solution() const {
    return _solution;
}

[[nodiscard]] const std::vector<int> &ProxiSolutionILSTS::non_empty_colors() const {
    return _solution.non_empty_colors();
}

[[nodiscard]] int ProxiSolutionILSTS::color(const int &vertex) const {
    return _solution.color(vertex);
}

[[nodiscard]] int ProxiSolutionILSTS::score_wvcp() const {
    return _solution.score_wvcp();
}

[[nodiscard]] int ProxiSolutionILSTS::max_weight(const int &color) const {
    return _solution.max_weight(color);
}

[[nodiscard]] bool ProxiSolutionILSTS::is_color_empty(const int color) const {
    return _solution.is_color_empty(color);
}

[[nodiscard]] int ProxiSolutionILSTS::conflicts_colors(const int &color,
                                                       const int &vertex) const {
    return _solution.conflicts_colors(color, vertex);
}

[[nodiscard]] int ProxiSolutionILSTS::nb_colors() const {
    return _solution.nb_colors();
}
