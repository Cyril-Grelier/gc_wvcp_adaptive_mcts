#pragma once

#include <vector>

#include "../representation/Solution.hpp"
#include "../representation/enum_types.hpp"
#include "neural_network.hpp"

/**
 * @brief Helper to select the next operator to use
 *
 * First create the helper, if its the Neural Net one,
 * make sure to update Solution::max_nb_colors before creating
 * the helper object to init the layer sizes of the NN
 *
 * 1a. For the general use, ask for an operator
 * with get_operator() -> operator number
 *
 * 1b. For the NN helper, feed the helper
 * with get_operator(solution) -> operator number
 *
 * Then
 * 2. Apply the operator
 * 3. Then give the information of the result to the helper
 * with update_obtained_solution(operator_number, score)
 * operator_number is the operator used to reach the score (in case you add randomness)
 * 4. After, call update_helper() to update its probabilities
 * 5. At the end of the turn, call increment_turn()
 *
 */
class AdaptiveHelper {
  protected:
    int nb_operators;
    int memory_size;
    std::vector<double> proba_operator;
    std::vector<double> utility;
    std::vector<int> past_operators;
    int turn{0};
    std::vector<double> normalized_utilities;
    std::vector<int> nb_times_selected;

    // number of times an operator have been selected since the beginning of the run
    std::vector<int> nb_times_used_total;
    // mean score
    std::vector<double> mean_score;
    std::set<int> possible_operators;
    std::set<int> removed_operators;

    virtual void compute_normalized_utilities_and_nb_selected();

  public:
    AdaptiveHelper(const int nb_operators_, const int memory_size_);

    virtual ~AdaptiveHelper() = default;

    /**
     * @brief Get the index of the most promising operator according to the helper
     *
     * @return int the operator
     */
    virtual int get_operator() = 0;

    /**
     * @brief Update information about the solution obtained
     *
     * @param operator_number operator selected
     * @param solution solution reached
     */
    virtual void update_obtained_solution(const int operator_number, const int score);

    /**
     * @brief update the adaptive helper
     *
     */
    virtual void update_helper();

    /**
     * @brief to call after update helper
     *
     */
    void increment_turn();

    std::string to_str_proba() const;
    std::string get_selected_str() const;
};

/**
 * @brief Select the one operator (do nothing)
 *
 */
class AdaptiveHelper_none : public AdaptiveHelper {
  public:
    AdaptiveHelper_none(const int nb_operator);

    virtual ~AdaptiveHelper_none() override = default;

    int get_operator() override;
};

/**
 * @brief Select each operator one after an other
 *
 */
class AdaptiveHelper_iterated : public AdaptiveHelper {
  public:
    AdaptiveHelper_iterated(const int nb_operator);

    virtual ~AdaptiveHelper_iterated() override = default;

    int get_operator() override;
};

/**
 * @brief Select next operator randomly
 *
 * From :
 * Goëffon, A., Lardeux, F., Saubion, F., 2016.
 * Simulating non-stationary operators in search algorithms.
 * Applied Soft Computing 38, 257–268.
 * https://doi.org/10.1016/j.asoc.2015.09.024
 *
 */
class AdaptiveHelper_random : public AdaptiveHelper {
  public:
    AdaptiveHelper_random(const int nb_operator);

    virtual ~AdaptiveHelper_random() override = default;

    int get_operator() override;
};

/**
 * @brief Delete operators during the search
 *
 */
class AdaptiveHelper_deleter : public AdaptiveHelper {

  public:
    AdaptiveHelper_deleter(const int nb_operator);

    virtual ~AdaptiveHelper_deleter() override = default;

    int get_operator() override;

    void update_helper() override;
};

/**
 * @brief Select next operator according to past results
 *
 * From :
 * Goëffon, A., Lardeux, F., Saubion, F., 2016.
 * Simulating non-stationary operators in search algorithms.
 * Applied Soft Computing 38, 257–268.
 * https://doi.org/10.1016/j.asoc.2015.09.024
 *
 */
class AdaptiveHelper_roulette_wheel : public AdaptiveHelper {
  public:
    AdaptiveHelper_roulette_wheel(const int nb_operator);

    virtual ~AdaptiveHelper_roulette_wheel() override = default;

    int get_operator() override;

    void update_helper() override;
};

/**
 * @brief Select next operator according to past results
 *
 * From :
 * Goëffon, A., Lardeux, F., Saubion, F., 2016.
 * Simulating non-stationary operators in search algorithms.
 * Applied Soft Computing 38, 257–268.
 * https://doi.org/10.1016/j.asoc.2015.09.024
 *
 */
class AdaptiveHelper_pursuit : public AdaptiveHelper {
  public:
    AdaptiveHelper_pursuit(const int nb_operator);

    virtual ~AdaptiveHelper_pursuit() override = default;

    int get_operator() override;

    void update_helper() override;
};

/**
 * @brief Select next operator according to past results
 *
 * From :
 * Goëffon, A., Lardeux, F., Saubion, F., 2016.
 * Simulating non-stationary operators in search algorithms.
 * Applied Soft Computing 38, 257–268.
 * https://doi.org/10.1016/j.asoc.2015.09.024
 *
 */
class AdaptiveHelper_ucb : public AdaptiveHelper {
  public:
    AdaptiveHelper_ucb(const int nb_operator);

    virtual ~AdaptiveHelper_ucb() override = default;

    int get_operator() override;

    void update_helper() override;
};

/**
 * @brief Select next operator according to the prediction of a neural network
 *
 */
class AdaptiveHelper_neural_net : public AdaptiveHelper {
    NeuralNetwork _model;
    torch::optim::Adam _optimizer;
    std::vector<torch::Tensor> _solutions;
    std::vector<torch::Tensor> _utility_operator;

  public:
    AdaptiveHelper_neural_net(const int nb_operator);

    virtual ~AdaptiveHelper_neural_net() override = default;

    /**
     * @brief don't use this function with AdaptiveHelper_neural_net
     * @return throw 1, use get_operator_nn
     */
    int get_operator() override;

    /**
     * @brief Return the next operator to use
     * This function add the given solution to the set of example
     *
     * @param solution the base solution to predict from
     * @return int operator number
     */
    int get_operator(const Solution &solution);

    /**
     * @brief Used to update _solutions and _utility_operator tensors
     */
    void update_obtained_solution(const int operator_number, const int score) override;

    void update_helper() override;
};

std::unique_ptr<AdaptiveHelper> get_adaptive_helper(const std::string &adaptive_type,
                                                    int nb_operators);
