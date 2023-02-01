#include "adaptive.hpp"

#include <algorithm>
#include <cmath>

#include "../utils/random_generator.hpp"
#include "../utils/utils.hpp"

AdaptiveHelper::AdaptiveHelper(const int nb_operators_, const int memory_size_)
    : nb_operators(nb_operators_),
      memory_size(memory_size_),
      proba_operator(nb_operators_, 1 / static_cast<double>(nb_operators_)),
      utility(memory_size_, 0),
      past_operators(memory_size_, -1),
      normalized_utilities(nb_operators_, 0),
      nb_times_selected(nb_operators_, 0),
      nb_times_used_total(nb_operators_, 0),
      mean_score(nb_operators_, 0),
      possible_operators() {
    for (int i = 0; i < nb_operators_; ++i) {
        possible_operators.insert(i);
    }
}

void AdaptiveHelper::update_obtained_solution(const int operator_number,
                                              const int score) {
    const int index = turn % memory_size;
    utility[index] = score;
    past_operators[index] = operator_number;
    mean_score[operator_number] =
        ((mean_score[operator_number] *
          static_cast<double>(nb_times_used_total[operator_number])) +
         score) /
        static_cast<double>(nb_times_used_total[operator_number] + 1);
    ++nb_times_used_total[operator_number];
}

void AdaptiveHelper::update_helper() {
}

void AdaptiveHelper::increment_turn() {
    ++turn;
}

std::string AdaptiveHelper::to_str_proba() const {
    return fmt::format("{:.2f}", fmt::join(proba_operator, ":"));
}

void AdaptiveHelper::compute_normalized_utilities_and_nb_selected() {
    std::fill(normalized_utilities.begin(), normalized_utilities.end(), 0);
    std::fill(nb_times_selected.begin(), nb_times_selected.end(), 0);

    // get the sum of past utilities
    for (size_t i = 0; i < utility.size(); ++i) {
        const int operator_selected = past_operators[i];
        if (operator_selected != -1) {
            ++nb_times_selected[operator_selected];
            normalized_utilities[operator_selected] += utility[i];
        }
    }

    // mean the utilities
    for (int o = 0; o < nb_operators; ++o) {
        if (nb_times_selected[o] != 0) {
            normalized_utilities[o] = normalized_utilities[o] / nb_times_selected[o];
        }
    }
    // if an operator have never been selected,
    // then its utility is the one of the worst operator
    const double worst{
        *std::max_element(normalized_utilities.begin(), normalized_utilities.end())};
    for (int o = 0; o < nb_operators; ++o) {
        if (nb_times_selected[o] == 0) {
            normalized_utilities[o] = worst;
        }
    }
    // normalization
    const auto minmax =
        std::minmax_element(normalized_utilities.begin(), normalized_utilities.end());
    const double min_val = *minmax.first;
    const double max_val = *minmax.second;
    if (min_val == max_val) {
        // if all the operators have the same mean score
        // they all get normalized to 1
        for (int o = 0; o < nb_operators; ++o) {
            normalized_utilities[o] = 1;
        }
    } else {
        // otherwise the utilities are normally normalized
        // in reverse as its a minimization problem
        for (int o = 0; o < nb_operators; ++o) {
            normalized_utilities[o] =
                (normalized_utilities[o] - max_val) / (min_val - max_val);
        }
    }
}

std::string AdaptiveHelper::get_selected_str() const {
    const int index = turn % memory_size;
    return std::to_string(past_operators[index]);
}

/************************************************************************
 *
 *                         AdaptiveHelper_none
 *
 ************************************************************************/

AdaptiveHelper_none::AdaptiveHelper_none(const int nb_operator)
    : AdaptiveHelper(nb_operator, 1) {
    proba_operator[0] = 1;
    proba_operator[1] = 0;
}

int AdaptiveHelper_none::get_operator() {
    return 0;
}

/************************************************************************
 *
 *                         AdaptiveHelper_iterated
 *
 ************************************************************************/

AdaptiveHelper_iterated::AdaptiveHelper_iterated(const int nb_operator)
    : AdaptiveHelper(nb_operator, 1) {
}

int AdaptiveHelper_iterated::get_operator() {
    const int operator_number = turn % nb_operators;
    return operator_number;
}

/************************************************************************
 *
 *                         AdaptiveHelper_random
 *
 ************************************************************************/

AdaptiveHelper_random::AdaptiveHelper_random(const int nb_operator)
    : AdaptiveHelper(nb_operator, 1) {
}

int AdaptiveHelper_random::get_operator() {
    std::discrete_distribution<> d(proba_operator.begin(), proba_operator.end());
    const int operator_number = d(rd::generator);
    return operator_number;
}

/************************************************************************
 *
 *                         AdaptiveHelper_deleter
 *
 ************************************************************************/

AdaptiveHelper_deleter::AdaptiveHelper_deleter(const int nb_operator)
    : AdaptiveHelper(nb_operator, 1) {
}

int AdaptiveHelper_deleter::get_operator() {
    const int operator_number = rd::choice(possible_operators);
    return operator_number;
}

void AdaptiveHelper_deleter::update_helper() {
    if ((turn < 5 * nb_operators) or (not(turn % 5 == 0)) or
        (possible_operators.size() == 1)) {
        return;
    }
    int worst_operator = *possible_operators.begin();
    for (const auto o : possible_operators) {
        if (mean_score[o] > mean_score[worst_operator]) {
            worst_operator = o;
        }
    }
    possible_operators.erase(worst_operator);
    removed_operators.insert(worst_operator);
}

/************************************************************************
 *
 *                         AdaptiveHelper_roulette_wheel
 *
 ************************************************************************/

AdaptiveHelper_roulette_wheel::AdaptiveHelper_roulette_wheel(const int nb_operator)
    : AdaptiveHelper(nb_operator, Parameters::p->window_size) {
}

int AdaptiveHelper_roulette_wheel::get_operator() {
    std::discrete_distribution<> d(proba_operator.begin(), proba_operator.end());
    const int operator_number = d(rd::generator);
    return operator_number;
}

void AdaptiveHelper_roulette_wheel::update_helper() {
    if (turn < 5 * nb_operators) {
        return;
    }

    compute_normalized_utilities_and_nb_selected();

    double sum_uk = sum(normalized_utilities);

    const double p_min = 1.0 / static_cast<double>(nb_operators * 5);
    for (auto o = 0; o < nb_operators; ++o) {
        proba_operator[o] =
            p_min + (1 - nb_operators * p_min) * (normalized_utilities[o] / sum_uk);
    }
}

/************************************************************************
 *
 *                         AdaptiveHelper_pursuit
 *
 ************************************************************************/

AdaptiveHelper_pursuit::AdaptiveHelper_pursuit(const int nb_operator)
    : AdaptiveHelper(nb_operator, Parameters::p->window_size) {
}

int AdaptiveHelper_pursuit::get_operator() {
    std::discrete_distribution<> d(proba_operator.begin(), proba_operator.end());
    const int operator_number = d(rd::generator);
    return operator_number;
}

void AdaptiveHelper_pursuit::update_helper() {
    if (turn < 5 * nb_operators) {
        return;
    }

    if (turn % 20 == 0) {
        // reset proba
        const double val = 1 / static_cast<double>(nb_operators);
        std::fill(proba_operator.begin(), proba_operator.end(), val);
    }

    const double p_min = 1.0 / static_cast<double>(nb_operators * 5);
    const double p_max = 1 - static_cast<double>(nb_operators - 1) * p_min;
    const double beta = 0.7;

    compute_normalized_utilities_and_nb_selected();

    for (int o = 0; o < nb_operators; ++o) {
        const double t_minus_1 = proba_operator[o];
        if (normalized_utilities[o] == 1) {
            // if its the best operator (normalization at 1)
            proba_operator[o] = t_minus_1 + beta * (p_max - t_minus_1);
        } else {
            proba_operator[o] = t_minus_1 + beta * (p_min - t_minus_1);
        }
    }
}

/************************************************************************
 *
 *                         AdaptiveHelper_ucb
 *
 ************************************************************************/

AdaptiveHelper_ucb::AdaptiveHelper_ucb(const int nb_operator)
    : AdaptiveHelper(nb_operator, Parameters::p->window_size) {
}

int AdaptiveHelper_ucb::get_operator() {
    const double maxi = *std::max_element(proba_operator.begin(), proba_operator.end());
    std::vector<int> bests;
    for (int o = 0; o < nb_operators; ++o) {
        if (proba_operator[o] == maxi) {
            bests.push_back(o);
        }
    }
    const int operator_number = rd::choice(bests);
    return operator_number;
}

void AdaptiveHelper_ucb::update_helper() {
    if (turn < 5 * nb_operators) {
        return;
    }
    compute_normalized_utilities_and_nb_selected();

    int size = memory_size;
    if (turn < memory_size) {
        size = (turn + 1);
    }

    // Exploration
    std::vector<double> exploration(nb_operators, 0);
    for (int o = 0; o < nb_operators; ++o) {
        exploration[o] = std::sqrt(2 * std::log(size) / (nb_times_selected[o] + 1));
    }
    // set the ucb score
    for (int o = 0; o < nb_operators; ++o) {
        proba_operator[o] =
            normalized_utilities[o] + Parameters::p->coeff_exploi_explo * exploration[o];
    }
}

/************************************************************************
 *
 *                         AdaptiveHelper_neural_net
 *
 ************************************************************************/

AdaptiveHelper_neural_net::AdaptiveHelper_neural_net(const int nb_operator)
    : AdaptiveHelper(nb_operator, Parameters::p->window_size),
      _model(Solution::max_nb_colors, Graph::g->nb_vertices, nb_operator),
      _optimizer(_model.parameters(), /*lr=*/0.001) {
    _solutions.reserve(memory_size);
    _utility_operator.reserve(memory_size);
}

int AdaptiveHelper_neural_net::get_operator() {
    fmt::print(stderr, "error use get_operator_nn for the neural net operator");
    throw 1;
}

int AdaptiveHelper_neural_net::get_operator(const Solution &solution) {
    auto tensor_solution = solution_to_tensor(solution);
    if (turn < memory_size) {
        _solutions.emplace_back(tensor_solution);
        _utility_operator.emplace_back(torch::tensor({-1, -1}));
    } else {
        const int index = turn % memory_size;
        _solutions[index] = tensor_solution;
        _utility_operator[index] = torch::tensor({-1, -1});
    }

    int operator_number;

    std::uniform_int_distribution<int> distribution(0, 99);
    if (distribution(rd::generator) < 20 or turn < 5 * nb_operators) {
        std::uniform_int_distribution<int> dist(
            0, static_cast<int>(proba_operator.size() - 1));
        operator_number = dist(rd::generator);
    } else {
        torch::Tensor prediction = _model.forward(tensor_solution.unsqueeze(0));
        // update the proba with the predictions
        for (int op = 0; op < nb_operators; ++op) {
            proba_operator[op] = prediction.index({0, op}).item().toDouble();
        }
        operator_number = prediction.argmin(1).item().toInt();
    }
    return operator_number;
}

void AdaptiveHelper_neural_net::update_obtained_solution(const int operator_number,
                                                         const int score) {
    AdaptiveHelper::update_obtained_solution(operator_number, score);
    const int index = turn % memory_size;
    _utility_operator[index] = torch::tensor(
        {static_cast<long>(utility[index]), static_cast<long>(past_operators[index])});
}

void AdaptiveHelper_neural_net::update_helper() {
    if (turn < 5 * nb_operators or turn % 10 != 0) {
        return;
    }
    // train the NN
    auto data_set = SolutionDataset(_solutions, _utility_operator)
                        .map(torch::data::transforms::Stack<>());
    auto batch_size = 20;
    auto data_loader =
        torch::data::make_data_loader<torch::data::samplers::RandomSampler>(
            std::move(data_set), batch_size);
    const int nb_epoch = 15;
    train(data_loader, _model, _optimizer, nb_epoch);
}

std::unique_ptr<AdaptiveHelper> get_adaptive_helper(const std::string &adaptive_type,
                                                    int nb_operators) {
    if (adaptive_type == "none")
        return std::make_unique<AdaptiveHelper_none>(nb_operators);
    if (adaptive_type == "iterated")
        return std::make_unique<AdaptiveHelper_iterated>(nb_operators);
    if (adaptive_type == "random")
        return std::make_unique<AdaptiveHelper_random>(nb_operators);
    if (adaptive_type == "deleter")
        return std::make_unique<AdaptiveHelper_deleter>(nb_operators);
    if (adaptive_type == "roulette_wheel")
        return std::make_unique<AdaptiveHelper_roulette_wheel>(nb_operators);
    if (adaptive_type == "pursuit")
        return std::make_unique<AdaptiveHelper_pursuit>(nb_operators);
    if (adaptive_type == "ucb")
        return std::make_unique<AdaptiveHelper_ucb>(nb_operators);
    if (adaptive_type == "neural_net")
        return std::make_unique<AdaptiveHelper_neural_net>(nb_operators);
    if (adaptive_type == "neural_net_cross")
        return std::make_unique<AdaptiveHelper_neural_net>(nb_operators);

    fmt::print(
        stderr,
        "Unknown adaptive : {}\n"
        "Please select : "
        "none, iterated, random, deleter, roulette_wheel, pursuit, ucb, neural_net, "
        "neural_net_cross\n",
        adaptive_type);
    exit(1);
}
