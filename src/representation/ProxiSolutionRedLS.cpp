#include "ProxiSolutionRedLS.hpp"

#include <cassert>

ProxiSolutionRedLS::ProxiSolutionRedLS(Solution solution)
    : _solution(solution),
      _conflicts_colors(solution.conflicts_colors()),
      _edge_weights(Graph::g->nb_vertices, std::vector<int>(Graph::g->nb_vertices, 0)) {
    // init _edge_weights
    for (const auto &[v1, v2] : Graph::g->edges_list) {
        _edge_weights[v1][v2] = 1;
        _edge_weights[v2][v1] = 1;
    }
}

int ProxiSolutionRedLS::add_to_color(const int vertex, const int proposed_color) {
    const int color = _solution.add_to_color(vertex, proposed_color);
    if (proposed_color != color) {
        _conflicts_colors.emplace_back(Graph::g->nb_vertices, 0);
    }

    if (_conflicts_colors[color][vertex] > 0) {
        // Update penalty
        _penalty += _conflicts_colors[color][vertex];
        for (const auto &neighbor : Graph::g->neighborhood[vertex]) {
            if (_solution.color(neighbor) == color) {
                const int lower = std::min(neighbor, vertex);
                const int higher = std::max(neighbor, vertex);
                _conflict_edges.emplace_back(std::make_tuple(lower, higher));
            }
        }
    }
    // update conflicts for neighbors
    for (const auto &neighbor : Graph::g->neighborhood[vertex]) {
        _conflicts_colors[color][neighbor] += _edge_weights[vertex][neighbor];
    }

    return color;
}

int ProxiSolutionRedLS::delete_from_color(const int vertex) {
    const int color = _solution.color(vertex);

    // Update nb of conflicts and list of conflicting edges
    if (_conflicts_colors[color][vertex] > 0) {
        _penalty -= _conflicts_colors[color][vertex];
        for (const auto &neighbor : Graph::g->neighborhood[vertex]) {
            if (_solution.color(neighbor) == color) {
                const int lower = std::min(neighbor, vertex);
                const int higher = std::max(neighbor, vertex);
                int index = 0;
                for (const auto &[edge1, edge2] : _conflict_edges) {
                    if (edge1 == lower and edge2 == higher) {
                        break;
                    }
                    index++;
                }
                _conflict_edges.erase(_conflict_edges.begin() + index);
            }
        }
    }

    // update conflicts for neighbors
    for (const int neighbor : Graph::g->neighborhood[vertex]) {
        // _conflicts_colors[color][neighbor]--;
        _conflicts_colors[color][neighbor] -= _edge_weights[vertex][neighbor];
    }

    return _solution.delete_from_color(vertex);
}

void ProxiSolutionRedLS::increment_edge_weights() {
    for (const auto &[edge1, edge2] : _conflict_edges) {
        ++_edge_weights[edge1][edge2];
        ++_edge_weights[edge2][edge1];
        ++_conflicts_colors[_solution.color(edge1)][edge2];
        ++_conflicts_colors[_solution.color(edge2)][edge1];
    }
    _penalty += static_cast<int>(_conflict_edges.size());
}

bool ProxiSolutionRedLS::check_solution() const {
    for (int vertex = 0; vertex < Graph::g->nb_vertices; ++vertex) {
        const int color = _solution.color(vertex);
        for (const auto &neighbor : Graph::g->neighborhood[vertex]) {
            if (_solution.color(neighbor) == color) {
                const int lower = std::min(neighbor, vertex);
                const int higher = std::max(neighbor, vertex);
                int index = 0;
                for (const auto &[edge1, edge2] : _conflict_edges) {
                    if (edge1 == lower and edge2 == higher) {
                        break;
                    }
                    index++;
                }
                assert(index < static_cast<int>(_conflict_edges.size()));
            }
        }
    }
    return _solution.check_solution();
}

[[nodiscard]] int ProxiSolutionRedLS::delta_conflicts(const int vertex,
                                                      const int color) const {
    return _conflicts_colors[color][vertex] -
           _conflicts_colors[_solution.color(vertex)][vertex];
}

[[nodiscard]] int ProxiSolutionRedLS::conflicts_colors(const int &color,
                                                       const int &vertex) const {
    return _conflicts_colors[color][vertex];
}

[[nodiscard]] int ProxiSolutionRedLS::penalty() const {
    return _penalty;
}

[[nodiscard]] std::vector<std::tuple<int, int>>
ProxiSolutionRedLS::conflict_edges() const {
    return _conflict_edges;
}

[[nodiscard]] Solution ProxiSolutionRedLS::solution() const {
    return _solution;
}

int ProxiSolutionRedLS::delta_wvcp_score(const int vertex, const int color) const {
    return _solution.delta_wvcp_score(vertex, color);
}

[[nodiscard]] const std::vector<int> &ProxiSolutionRedLS::non_empty_colors() const {
    return _solution.non_empty_colors();
}

[[nodiscard]] int ProxiSolutionRedLS::color(const int &vertex) const {
    return _solution.color(vertex);
}

[[nodiscard]] int ProxiSolutionRedLS::score_wvcp() const {
    return _solution.score_wvcp();
}

[[nodiscard]] bool ProxiSolutionRedLS::has_conflicts(const int vertex) const {
    return _solution.has_conflicts(vertex);
}

[[nodiscard]] const std::set<int> &
ProxiSolutionRedLS::colors_vertices(const int &color) const {
    return _solution.colors_vertices(color);
}

[[nodiscard]] int ProxiSolutionRedLS::max_weight(const int &color) const {
    return _solution.max_weight(color);
}

[[nodiscard]] int ProxiSolutionRedLS::first_free_vertex() const {
    return _solution.first_free_vertex();
}
