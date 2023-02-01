#include "SimulationHelper.hpp"

#include "../utils/random_generator.hpp"

void SimulationHelper::accept_solution(const Solution &solution) {
    past_solutions.push_back(solution.colors());
    past_nb_colors.push_back(solution.nb_colors());
    fit_condition = std::min(solution.score_wvcp(), fit_condition);
}

bool SimulationHelper::distant_enough(const Solution &solution) {
    bool distant_enough = true;
    const auto &colors = solution.colors();
    const auto &nb_colors = solution.nb_colors();
    for (size_t i = 0; i < past_solutions.size() and distant_enough; ++i) {
        const int dist = distance_approximation(
            past_solutions[i], past_nb_colors[i], colors, nb_colors);
        if (dist < distance_min) {
            return false;
        }
    }
    return true;
}

bool SimulationHelper::score_low_enough(const Solution &solution) {
    const int min_score =
        std::max(static_cast<int>(fit_condition * 1.01), fit_condition + 1);
    const bool score_low_enough = solution.score_wvcp() <= min_score;
    return score_low_enough;
}

bool SimulationHelper::level_ok(const Solution &solution) {
    const int v = solution.first_free_vertex();
    const int level = v % depth_min;
    const bool level_ok = v > depth_min and (level == 0);
    return level_ok;
}

bool SimulationHelper::depth_chance_ok(const Solution &solution) {
    // the local search can only be launch between 5 and 90 % of the tree
    std::uniform_int_distribution<int> distribution(5, 95);
    const int percentage_already_colored =
        (solution.first_free_vertex() * 100) / Graph::g->nb_vertices;
    const int chance_of_passing = distribution(rd::generator);
    const bool depth_chance_ok = percentage_already_colored >= chance_of_passing;
    return depth_chance_ok;
}

simulation_ptr get_simulation_fct(const std::string &simulation) {
    if (simulation == "no_ls") {
        return no_ls;
    }
    if (simulation == "always_ls") {
        return always_ls;
    }
    if (simulation == "fit") {
        return fit;
    }
    if (simulation == "depth") {
        return depth;
    }
    if (simulation == "level") {
        return level;
    }
    if (simulation == "depth_fit") {
        return depth_fit;
    }
    if (simulation == "chance") {
        return chance;
    }
    fmt::print(stderr,
               "Unknown simulation : {}\n"
               "Please select : "
               "no_ls, always_ls, fit, depth, level, depth_fit, chance\n",
               simulation);
    exit(1);
}

bool no_ls(const Solution &solution, SimulationHelper &helper) {
    (void)solution;
    (void)helper;
    return false;
}

bool always_ls(const Solution &solution, SimulationHelper &helper) {
    (void)solution;
    (void)helper;
    return true;
}

bool fit(const Solution &solution, SimulationHelper &helper) {
    if (not helper.score_low_enough(solution)) {
        return false;
    }

    if (not helper.distant_enough(solution)) {
        return false;
    }

    helper.accept_solution(solution);
    return true;
}

bool depth(const Solution &solution, SimulationHelper &helper) {

    if (not helper.depth_chance_ok(solution)) {
        return false;
    }

    if (not helper.distant_enough(solution)) {
        return false;
    }

    helper.accept_solution(solution);
    return true;
}

bool level(const Solution &solution, SimulationHelper &helper) {

    if (not helper.level_ok(solution)) {
        return false;
    }

    if (not helper.distant_enough(solution)) {
        return false;
    }

    helper.accept_solution(solution);
    return true;
}

bool depth_fit(const Solution &solution, SimulationHelper &helper) {
    if (not helper.score_low_enough(solution)) {
        return false;
    }

    if (not helper.depth_chance_ok(solution)) {
        return false;
    }

    if (not helper.distant_enough(solution)) {
        return false;
    }

    helper.accept_solution(solution);
    return true;
}

bool chance(const Solution &solution, SimulationHelper &helper) {
    std::uniform_int_distribution<int> distribution(0, 99);
    const int chance_of_passing = distribution(rd::generator);
    if (chance_of_passing < 5) {
        return false;
    }
    helper.accept_solution(solution);
    return true;
}