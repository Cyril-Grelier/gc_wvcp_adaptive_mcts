#pragma once

#include <string>
#include <vector>

// import fmt here to avoid importing it everywhere
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Weffc++"
#include <fmt/printf.h>
#pragma GCC diagnostic pop

/**
 * @brief Represent the action of moving a vertex to a color that giving a score
 *
 */
struct Action {
    /** @brief vertex to color*/
    int vertex;
    /** @brief color to use*/
    int color;
    /** @brief next score*/
    int score;
};

/**
 * @brief Action are the same if they share the same values
 *
 * @param m1 first action
 * @param m2 second action
 * @return true m1 is equal to m2
 * @return false m1 is not equal to m2
 */
bool operator==(const Action &m1, const Action &m2);

/**
 * @brief Compare actions to sort it
 *
 * @param a first action
 * @param b second action
 * @return true a has a higher score than b
 * @return false a has a lower or equal score than b
 */
bool compare_actions(const Action &a, const Action &b);

/**
 * @brief Represent the action of moving a vertex to a color
 *
 */
struct Coloration {
    /** @brief vertex to color*/
    int vertex;
    /** @brief color to use*/
    int color;
};

/** @brief Perturbations for AFISA*/
enum class Perturbation
{
    no_perturbation,
    unlimited,
    no_tabu
};
