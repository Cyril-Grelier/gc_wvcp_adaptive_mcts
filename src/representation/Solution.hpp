#pragma once

#include <memory>
#include <set>
#include <tuple>

#include "Graph.hpp"
#include "Parameters.hpp"

/**
 * @brief Representation of a solution for Weighted Vertex Coloring Problem
 *
 */
class Solution {
  public:
    /** @brief WVCP best found score*/
    static int best_score_wvcp;
    /** @brief Minimal nb of color found (you have to update it)*/
    static int best_nb_colors;
    /** @brief Max number of color for fixed nb_colors methods (you have to update it) */
    static int max_nb_colors;
    /** @brief Header csv*/
    const static std::string header_csv;

  private:
    /** @brief For each vertex, its color*/
    std::vector<int> _colors{};
    /** @brief For each color, set of the vertices colored with the color*/
    std::vector<std::set<int>> _colors_vertices{};
    /** @brief For each color, the heaviest weight*/
    std::vector<int> _heaviest_weight{};

    /** @brief For each color, for each vertex, number of neighbors in the color*/
    std::vector<std::vector<int>> _conflicts_colors{};

    /** @brief number of opened colors (not automaticaly all used)*/
    int _nb_colors{0};
    /** @brief List of used colors*/
    std::vector<int> _non_empty_colors{};
    /** @brief List of unused colors*/
    std::vector<int> _empty_colors{};

    /** @brief Next vertex to color in the MCTS tree*/
    int _first_free_vertex{0};

    /** @brief WVCP score*/
    int _score_wvcp{0};

    /** @brief number of conflicts in the current solution (conflicting edges)*/
    int _penalty{0};
    /** @brief number of conflicting vertices*/
    int _nb_conflicting_vertices{0};

  public:
    /**
     * @brief Construct a new Solution object
     *
     */
    Solution();

    /**
     * @brief Destroy the Solution object
     *
     */
    ~Solution() = default;

    /**
     * @brief Color the vertex to the color
     * if the color is not created, the color is created
     * if the vertex is already colored, the vertex is uncolored before colored
     * with the color
     *
     * @param vertex the vertex to color
     * @param color the color to use (-1 to ask for a new color)
     * @return int the color used
     */
    int add_to_color(const int vertex, int color);

    /**
     * @brief Remove color of a vertex
     *
     * @param vertex vertex to modify
     * @return int the old color
     */
    int delete_from_color(const int vertex);

    /**
     * @brief Give the first available color for the given vertex,
     * -1 if no color available
     *
     * @param vertex the vertex
     * @return int the first available color
     */
    [[nodiscard]] int first_available_color(const int &vertex);

    [[nodiscard]] std::vector<int> available_colors(const int &vertex) const;

    void clean_conflicts();

    /**
     * @brief Remove one color and create conflicts
     *
     */
    void remove_one_color_and_create_conflicts();

    /**
     * @brief Compute the difference on the score if the vertex is colored with the color
     *
     * @param vertex the vertex to color
     * @param color the color to use
     * @return int difference on the score
     */
    [[nodiscard]] int delta_wvcp_score(const int vertex, const int color) const;

    /**
     * @brief Compute the difference on the score if the vertex lost its color
     *
     * @param vertex the vertex to check
     * @return int the difference on the score
     */
    [[nodiscard]] int delta_wvcp_score_old_color(const int vertex) const;

    /**
     * @brief Compute the difference on the conflicts if the vertex is colored with the
     * color
     *
     * @param vertex the vertex to use
     * @param color the color to use
     * @return int the difference on the number of conflicts
     */
    [[nodiscard]] int delta_conflicts(const int vertex, const int color) const;

    /**
     * @brief Increment the next vertex to color in the MCTS tree
     *
     */
    void increment_first_free_vertex();

    /**
     * @brief all used colors are the firsts ones
     *
     */
    void reorganize_colors();

    /**
     * @brief Check the validity of the solution
     *
     * @return true the solution is valid
     * @return false the solution is invalid
     */
    bool check_solution() const;

    /**
     * @brief Return max weight of the color
     *
     * @param color the color
     * @return int the max weight
     */
    [[nodiscard]] int max_weight(const int &color) const;

    /**
     * @brief Return the second max weight of the color
     *
     * @param color the color
     * @return int the second max weight
     */
    [[nodiscard]] int second_max_weight(const int &color) const;

    /**
     * @brief Return whether the vertex has conflict or not
     *
     * @param vertex the vertex
     * @return true there is conflicts
     * @return false there is no conflicts
     */
    [[nodiscard]] bool has_conflicts(const int vertex) const;

    /**
     * @brief Return true if the color is empty
     *
     * @return true no vertex in the color
     * @return false vertex in the color
     */
    [[nodiscard]] bool is_color_empty(const int color) const;

    /**
     * @brief Return the solution in csv format
     *
     * @return std::string the solution in csv format
     */
    [[nodiscard]] std::string line_csv() const;

    /**
     * @brief Return the colors of the vertices
     *
     * @return const std::vector<int> colors
     */
    [[nodiscard]] const std::vector<int> &colors() const;

    /**
     * @brief Return the color of a vertex
     *
     * @param vertex the vertex
     * @return int the color of the vertex
     */
    [[nodiscard]] int color(const int &vertex) const;

    /**
     * @brief Return number of colors
     *
     * @return int number of colors
     */
    [[nodiscard]] int nb_colors() const;

    /**
     * @brief Gives the score of the solution
     *
     * @return int the score
     */
    [[nodiscard]] int score_wvcp() const;

    /**
     * @brief Return number of conflicts between vertices
     *
     * @return int penalty
     */
    [[nodiscard]] int penalty() const;

    /**
     * @brief return the number of conflicts on the color for the vertex
     *
     * @return int number of conflicts
     */
    [[nodiscard]] int conflicts_colors(const int &color, const int &vertex) const;

    /**
     * @brief Return vertices in given color
     *
     * @param color given color
     * @return std::set<int> vertices in the color
     */
    [[nodiscard]] const std::set<int> &colors_vertices(const int &color) const;

    /**
     * @brief Return non empty colors
     *
     * @return const std::vector<int>& non empty colors
     */
    [[nodiscard]] const std::vector<int> &non_empty_colors() const;

    /**
     * @brief Get the number of the next vertex to color in the MCTS tree
     *
     * @return int next vertex
     */
    [[nodiscard]] int first_free_vertex() const;

    /**
     * @brief Get the number of non_empty colors
     *
     * @return long number of colors currently used
     */
    [[nodiscard]] long nb_non_empty_colors() const;

    /**
     * @brief Return number of vertices per color
     *
     * @param nb_colors_max the vector size will be of size nb_color_max
     * @return std::vector<int> number of vertices per color
     */
    [[nodiscard]] std::vector<int> nb_vertices_per_color(const int nb_colors_max) const;

    /**
     * @brief Return colors weights (after computing it)
     *
     * @return const std::vector<std::vector<int>>& colors_weights of the solution
     */
    [[nodiscard]] std::vector<std::vector<int>> weights() const;

    [[nodiscard]] const std::vector<std::vector<int>> &conflicts_colors() const;

    int nb_conflicting_vertices() const;
};

/**
 * @brief Compute an approximation of the distance between two solutions
 *
 * @param sol1 first solution
 * @param sol2 second solution
 * @return int distance
 */
[[nodiscard]] int distance_approximation(const Solution &sol1, const Solution &sol2);

/**
 * @brief Compute an approximation of the distance between two solutions
 *
 * @param col1 vector of colors sol 1
 * @param max_col1 max nb colors sol 1
 * @param col2 vector of colors sol 1
 * @param max_col2  max nb colors sol 1
 * @return int distance
 */
[[nodiscard]] int distance_approximation(const std::vector<int> &col1,
                                         const int max_col1,
                                         const std::vector<int> &col2,
                                         const int max_col2);

/**
 * @brief Compute the distance between two solutions
 *
 * @param sol1 first solution
 * @param sol2 second solution
 * @return int distance
 */
[[nodiscard]] int distance(const Solution &sol1, const Solution &sol2);
