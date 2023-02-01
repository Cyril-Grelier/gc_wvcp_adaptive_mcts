#pragma once

#include "Solution.hpp"

/**
 * @brief Proxi solution for RedLS
 *
 * Add conflicts on edges and other way to manage of conflicts on colors
 *
 */
class ProxiSolutionRedLS {

    /** @brief the current solution*/
    Solution _solution;
    /** @brief number of conflicts in the current solution*/
    int _penalty{0};

    /** @brief For each color, for each vertex, number of neighbors in the color*/
    std::vector<std::vector<int>> _conflicts_colors{};

    /** @brief list of conflicting edges in the solution*/
    std::vector<std::tuple<int, int>> _conflict_edges{};
    /** @brief edge weights (for RedLS)*/
    std::vector<std::vector<int>> _edge_weights{};

  public:
    ProxiSolutionRedLS() = delete;
    ProxiSolutionRedLS(Solution solution);

    int add_to_color(const int vertex, const int proposed_color);

    int delete_from_color(const int vertex);

    void increment_edge_weights();

    bool check_solution() const;

    [[nodiscard]] int delta_conflicts(const int vertex, const int color) const;

    [[nodiscard]] int conflicts_colors(const int &color, const int &vertex) const;

    [[nodiscard]] int penalty() const;

    [[nodiscard]] std::vector<std::tuple<int, int>> conflict_edges() const;

    [[nodiscard]] Solution solution() const;

    // Getters to original solution

    int delta_wvcp_score(const int vertex, const int color) const;

    [[nodiscard]] const std::vector<int> &non_empty_colors() const;

    [[nodiscard]] int color(const int &vertex) const;

    [[nodiscard]] int score_wvcp() const;

    [[nodiscard]] bool has_conflicts(const int vertex) const;

    [[nodiscard]] const std::set<int> &colors_vertices(const int &color) const;

    [[nodiscard]] int max_weight(const int &color) const;

    [[nodiscard]] int first_free_vertex() const;
};
