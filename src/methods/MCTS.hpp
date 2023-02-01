#pragma once

#include "../representation/Method.hpp"
#include "../representation/Node.hpp"
#include "../representation/Parameters.hpp"
#include "../representation/Solution.hpp"
#include "LocalSearch.hpp"
#include "SimulationHelper.hpp"
#include "adaptive.hpp"

/**
 * @brief Method for Monte Carlo Tree Search
 *
 */
class MCTS : public Method {
  private:
    /** @brief Root node of the MCTS*/
    std::shared_ptr<Node> _root_node;
    /** @brief Current node*/
    std::shared_ptr<Node> _current_node;
    /** @brief Solution at the beginning of the tree (will be copied at each turn)*/
    Solution _base_solution;
    /** @brief Best found solution*/
    Solution _best_solution;
    /** @brief Current solution*/
    Solution _current_solution;
    /** @brief Current turn of MCTS*/
    long _turn{-1};
    /** @brief Time before founding best score*/
    std::chrono::high_resolution_clock::time_point _t_best{};
    /** @brief Function to call for initialize the solutions*/
    init_ptr _initialization;
    /** @brief Function to call as local search*/
    std::vector<local_search_ptr> _local_search;
    /** @brief Function to call for simulation*/
    simulation_ptr _simulation;

    /** @brief Helper to choose the next pair of operator*/
    std::unique_ptr<AdaptiveHelper> _adaptive_helper{};

  public:
    /**
     * @brief Construct a new MCTS object
     *
     */
    explicit MCTS();

    ~MCTS();

    /**
     * @brief Stopping condition for the MCTS search depending on turn, time limit and the
     * tree fully explored or not
     *
     * @return true continue the search
     * @return false stop the search
     */
    bool stop_condition() const;

    /**
     * @brief Run the 4 phases of MCTS algorithm until stop condition
     *
     */
    void run() override;

    /**
     * @brief Selection phase of the MCTS algorithm
     *
     */
    void selection();

    /**
     * @brief Expansion phase of the MCTS algorithm
     *
     */
    void expansion();

    /**
     * @brief Return string of the MCTS csv format
     *
     * @return std::string header for csv file
     */
    [[nodiscard]] const std::string header_csv() const override;

    /**
     * @brief Return string of a line of the MCTS csv format
     *
     * @return std::string line for csv file
     */
    [[nodiscard]] const std::string line_csv() const override;

    /**
     * @brief Convert the tree in dot format into a file
     *
     * @param file_name name of the file
     */
    void to_dot(const std::string &file_name) const;
};

/**
 * @brief Give the next possible moves with the current placement of vertices
 *
 * @return std::vector<Action> List of next moves
 */
std::vector<Action> next_possible_moves(const Solution &solution);

/**
 * @brief Apply a move to the solution
 *
 * @param mv move to apply
 */
void apply_action(Solution &solution, const Action &action);
