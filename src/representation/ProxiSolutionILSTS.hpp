#pragma once

#include "Solution.hpp"

/**
 * @brief Proxi solution for ILSTS
 *
 * Add unassigned vertices
 *
 */
class ProxiSolutionILSTS {

    /** @brief the current solution*/
    Solution _solution;

    /** @brief For each vertex,
     * number of color the vertex can take without increasing the score*/
    std::vector<int> _nb_free_colors{};
    /** @brief WVCP Score when some vertices are unassigned*/
    int _unassigned_score{0};
    /** @brief List of unassigned vertices*/
    std::vector<int> _unassigned{};

  public:
    ProxiSolutionILSTS() = delete;
    ProxiSolutionILSTS(Solution solution);

    bool check_solution() const;

    /**
     * @brief Color the vertex to the color
     * if the color is not created, the color is created
     * if the vertex is already colored, the vertex is uncolored before colored
     * with the color
     *
     * @param vertex the vertex to color
     * @param color the color to use
     * @return int the color used (may be different from the given color if the color was
     * empty)
     */
    int add_to_color(const int vertex, const int color_proposed);

    /**
     * @brief Remove color of a vertex
     *
     * @param vertex vertex to modify
     * @return int the old color
     */
    int delete_from_color(const int vertex);

    /**
     * @brief Gives the unassigned score of the solution
     *
     * @return int the unassigned score
     */
    [[nodiscard]] int unassigned_score() const;

    /**
     * @brief Delete heaviest vertices from force colors
     *
     * @param force Number of deletions
     */
    void unassigned_random_heavy_vertices(const int force);

    /**
     * @brief Do a grenade operator on force random vertices
     *
     * @param force number of time the grenade operator is applied
     */
    void perturb_vertices(const int force);

    /**
     * @brief Assigned vertices to color randomly
     *
     * @param vertices vertices to color, the list is modified
     */
    void random_assignment(std::vector<int> &vertices);

    /**
     * @brief Assigned color to vertex randomly without increasing the score
     *
     * @param vertex the vertex to color
     * @return true the vertex has been colored without increasing the score
     * @return false there is no color available to color the vertex without increasing
     * the score
     */
    bool random_assignment_constrained(const int vertex);

    /**
     * @brief Assigned color to the vertices randomly without increasing the score
     *
     * @param vertices vertices to color, the list is modified, if the list isn't empty
     * after the function, the vertices left haven't free colors
     */
    void random_assignment_constrained(std::vector<int> &vertices);

    /**
     * @brief Gives the score of the solution or unassigned score if incomplete
     *
     * @return int the score maybe unassigned
     */
    [[nodiscard]] int get_score() const;

    /**
     * @brief Return unassigned vertices
     *
     * @return const std::vector<int>& unassigned vertices
     */
    [[nodiscard]] const std::vector<int> &unassigned() const;

    /**
     * @brief Return the number of color where the vertex can be moved without
     * incrementing the score
     *
     * @param vertex the vertex
     * @return int the number of color free
     */
    [[nodiscard]] int nb_free_colors(const int &vertex) const;

    /**
     * @brief Return is there is or not unassigned vertices
     *
     * @return true there is no unassigned vertices
     * @return false there is unassigned vertices
     */
    [[nodiscard]] bool has_unassigned_vertices() const;

    /**
     * @brief Add a vertex to the unassigned list
     *
     * @param vertex vertex to add
     */
    void add_unassigned_vertex(const int &vertex);

    /**
     * @brief Remove the given vertex from the unassigned list
     *
     * @param vertex vertex to remove
     */
    void remove_unassigned_vertex(const int &vertex);

    [[nodiscard]] Solution solution() const;

    // Getters to original solution

    [[nodiscard]] const std::vector<int> &non_empty_colors() const;

    [[nodiscard]] int color(const int &vertex) const;

    [[nodiscard]] int score_wvcp() const;

    [[nodiscard]] int max_weight(const int &color) const;

    [[nodiscard]] bool is_color_empty(const int color) const;

    [[nodiscard]] int conflicts_colors(const int &color, const int &vertex) const;

    [[nodiscard]] int nb_colors() const;
};
