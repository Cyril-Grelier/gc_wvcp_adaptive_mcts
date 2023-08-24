#pragma once
#include <algorithm>
#include <numeric>
#include <string>
#include <vector>

#include "../representation/Solution.hpp"

/**
 * @brief Search for the element in a sorted vector
 *
 * @tparam T value type
 * @param vector container
 * @param v value to find
 * @return true the value is in the vector
 * @return false the value is not in the vector
 */
template <class T> bool contains(const std::vector<T> &vector, const T &v) {
    auto it =
        std::lower_bound(vector.begin(), vector.end(), v, [](const T &l, const T &r) {
            return l < r;
        });
    return it != vector.end() && *it == v;
}

/**
 * @brief Insert element in sorted vector
 *
 * @tparam T value type
 * @param vector container
 * @param value value to add
 */
template <typename T> void insert_sorted(std::vector<T> &vector, T const &value) {
    if (vector.empty()) {
        vector.emplace_back(value);
    } else {
        vector.insert(std::upper_bound(std::begin(vector), std::end(vector), value),
                      value);
    }
}

/**
 * @brief Erase element from sorted vector
 *
 * @tparam T value type
 * @param vector container
 * @param value value to delete
 */
template <typename T> void erase_sorted(std::vector<T> &vector, T const &value) {
    auto lb = std::lower_bound(std::begin(vector), std::end(vector), value);
    if (lb != std::end(vector) and *lb == value) {
        vector.erase(lb);
    }
}

/**
 * @brief Get the index of the max element according to comp in the list of list
 *
 * @tparam T value type
 * @tparam Compare function(vector v1, vector v2) -> bool
 * @param vector container of container
 * @param comp function comparing vectors
 * @return int index of max value
 */
template <typename T, class Compare>
int get_index_max_element(std::vector<std::vector<T>> &vector, Compare comp) {
    return static_cast<int>(std::distance(
        vector.begin(), std::max_element(vector.begin(), vector.end(), comp)));
}

/**
 * @brief Return the sum of a vector
 *
 * @tparam T value Type
 * @param vector container
 * @return T sum
 */
template <typename T> T sum(const std::vector<T> &vector) {
    return std::reduce(vector.begin(), vector.end());
}

/**
 * @brief Return the mean of a vector
 *
 * @tparam T value type
 * @param vector container
 * @return float mean
 */
template <typename T> float mean(const std::vector<T> &vector) {
    return static_cast<float>(sum(vector)) / static_cast<float>(vector.size());
}

/**
 * @brief Split the string (copied) with the given delimiter
 *
 * @return std::vector<std::string>
 */
std::vector<std::string> split_string(std::string to_split, const std::string &delimiter);

/**
 * @brief Get the date format "%Y-%m-%d %H:%M:%S"
 *
 * @return std::string the date
 */
std::string get_date_str();

/**
 * @brief Print results for local search
 *
 * @param best_time
 * @param solution
 * @param turn
 */
void print_result_ls(const int64_t &best_time,
                     const Solution &solution,
                     const long &turn);
