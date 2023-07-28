#include "LocalSearch.hpp"

#include "../utils/utils.hpp"
#include "afisa.hpp"
#include "afisa_original.hpp"
#include "hill_climbing.hpp"
#include "ilsts.hpp"
#include "none_ls.hpp"
#include "partial_col.hpp"
#include "redls.hpp"
#include "redls_freeze.hpp"
#include "tabu_col.hpp"
#include "tabu_weight.hpp"
#include "useless_ls.hpp"
#include "worst_ls.hpp"

LocalSearch::LocalSearch()
    : _best_solution(),
      _init_function(get_initialization_fct(Parameters::p->initialization)),
      _local_search_function(get_local_search_fct(Parameters::p->local_search[0])) {
    _init_function(_best_solution);
}

void LocalSearch::run() {
    fmt::print(Parameters::p->output, "{}", header_csv());
    fmt::print(Parameters::p->output, "{}", line_csv());
    if (_local_search_function) {
        _local_search_function(_best_solution, true);
    }
}

[[nodiscard]] const std::string LocalSearch::header_csv() const {
    return fmt::format("turn,time,{}\n", Solution::header_csv);
}

[[nodiscard]] const std::string LocalSearch::line_csv() const {
    return fmt::format(
        "0,{},{}\n",
        Parameters::p->elapsed_time(std::chrono::high_resolution_clock::now()),
        _best_solution.line_csv());
}

local_search_ptr get_local_search_fct(const std::string &local_search) {
    if (local_search == "none")
        return none_ls;
    if (local_search == "hill_climbing")
        return hill_climbing_one_move;
    if (local_search == "tabu_col")
        return tabu_col;
    if (local_search == "tabu_col_neighborhood")
        return tabu_col_neighborhood;
    if (local_search == "random_walk_gcp")
        return random_walk_gcp;
    if (local_search == "partial_col")
        return partial_col;
    if (local_search == "tabu_weight")
        return tabu_weight;
    if (local_search == "tabu_weight_neighborhood")
        return tabu_weight_neighborhood;
    if (local_search == "random_walk_wvcp")
        return random_walk_wvcp;
    if (local_search == "afisa")
        return afisa;
    if (local_search == "afisa_original")
        return afisa_original;
    if (local_search == "redls")
        return redls;
    if (local_search == "redls_freeze")
        return redls_freeze;
    if (local_search == "ilsts")
        return ilsts;
    if (local_search == "useless_ls")
        return useless_ls;
    if (local_search == "worst_ls")
        return worst_ls;

    fmt::print(
        stderr,
        "Unknown local_search, please select : "
        "none, hill_climbing, tabu_col, partial_col, tabu_weight, "
        "afisa, afisa_original, redls, redls_freeze, ilsts,useless_ls, worst_ls\n");
    exit(1);
}
