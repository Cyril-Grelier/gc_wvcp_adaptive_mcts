#include "enum_types.hpp"

#include "../utils/utils.hpp"

bool operator==(const Action &m1, const Action &m2) {
    return m1.color == m2.color and m1.vertex == m2.vertex and m1.score == m2.score;
}

bool compare_actions(const Action &a, const Action &b) {
    return (a.score > b.score) or (a.score == b.score and a.color > b.color);
}
