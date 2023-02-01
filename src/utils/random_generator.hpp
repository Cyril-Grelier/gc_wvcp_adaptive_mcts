#pragma once

#include <random>
#include <set>
#include <vector>

namespace rd {
/** @brief random number generator*/
extern std::mt19937 generator;

/**
 * @brief Get the random value from a non empty container
 *
 * @tparam T type of the value
 * @param vector container
 * @return T random element from the vector
 */
template <typename T> T choice(const std::vector<T> &vector) {
    auto begin = vector.begin();
    std::uniform_int_distribution<> dis(
        0, static_cast<int>(std::distance(begin, vector.end())) - 1);
    std::advance(begin, dis(generator));
    return *begin;
}
template <typename T> T choice(const std::set<T> &set) {
    auto begin = set.begin();
    std::uniform_int_distribution<> dis(
        0, static_cast<int>(std::distance(begin, set.end())) - 1);
    std::advance(begin, dis(generator));
    return *begin;
}

} // namespace rd
