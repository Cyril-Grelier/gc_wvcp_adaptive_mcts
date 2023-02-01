#include "MCTS.hpp"

#include <cassert>
#include <fstream>
#include <iomanip>
#include <utility>

#include "../utils/random_generator.hpp"
#include "../utils/utils.hpp"

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Weffc++"
#include <fmt/printf.h>
#pragma GCC diagnostic pop

MCTS::MCTS()
    : _root_node(nullptr),
      _current_node(_root_node),
      _base_solution(),
      _best_solution(),
      _current_solution(_base_solution),
      _turn(0),
      _initialization(get_initialization_fct(Parameters::p->initialization)),
      _simulation(get_simulation_fct(Parameters::p->simulation)) {
    greedy_random(_best_solution);
    _t_best = std::chrono::high_resolution_clock::now();
    Solution::best_score_wvcp = _best_solution.score_wvcp();

    if (Parameters::p->use_target and Parameters::p->target > 0) {
        Solution::best_score_wvcp = Parameters::p->target;
    }
    // Creation of the base solution and root node
    const auto next_moves = next_possible_moves(_base_solution);
    assert(next_moves.size() == 1);
    apply_action(_base_solution, next_moves[0]);
    const auto next_possible_actions = next_possible_moves(_base_solution);
    _root_node = std::make_shared<Node>(nullptr, next_moves[0], next_possible_actions);

    fmt::print(Parameters::p->output, "{}", header_csv());

    // Prepare adaptive helper
    // make sure the neural net will get enough colors
    Solution::max_nb_colors = ((_best_solution.nb_colors() + Graph::g->nb_vertices) / 2);
    // get list of operators
    std::string operators_str = "";
    for (const auto &ls : Parameters::p->local_search) {
        if (operators_str == "") {
            operators_str += ls;
        } else {
            operators_str += ":" + ls;
        }
        _local_search.emplace_back(get_local_search_fct(ls));
    }

    if (Parameters::p->simulation != "no_ls") {
        // init the adaptive helper
        _adaptive_helper =
            get_adaptive_helper(Parameters::p->adaptive,
                                static_cast<int>(Parameters::p->local_search.size()));
        fmt::print(Parameters::p->output_tbt, "#operators\n");
        fmt::print(Parameters::p->output_tbt, "#{}\n", operators_str);
        fmt::print(Parameters::p->output_tbt,
                   "time,turn,proba,selected,score_pre_ls,score_post_ls\n");
    } else {
        _local_search.clear();
        _adaptive_helper = nullptr;
    }
}

MCTS::~MCTS() {
    _current_node = nullptr;
    _root_node->clean_graph(0);
    _root_node = nullptr;
}

bool MCTS::stop_condition() const {
    return (_turn < Parameters::p->nb_max_iterations) and
           (not Parameters::p->time_limit_reached()) and
           not(Parameters::p->objective == "reached" and
               (_best_solution.score_wvcp() <= Parameters::p->target)) and
           not _root_node->fully_explored();
}

void MCTS::run() {
    SimulationHelper helper;
    int operator_number = 0;
    auto *cast_nn = dynamic_cast<AdaptiveHelper_neural_net *>(_adaptive_helper.get());

    while (stop_condition()) {
        ++_turn;

        _current_node = _root_node;
        _current_solution = _base_solution;

        selection();

        expansion();

        // simulation
        _initialization(_current_solution);
        const int score_before_ls = _current_solution.score_wvcp();

        // local search or not and adaptive selection
        const bool use_local_search = _simulation(_current_solution, helper);
        if (use_local_search) {
            // ask the adaptive helper which local search to use
            if (cast_nn) {
                operator_number = cast_nn->get_operator(_current_solution);
            } else {
                operator_number = _adaptive_helper->get_operator();
            }
            const auto ls = _local_search[operator_number];
            ls(_current_solution, false);

            _adaptive_helper->update_obtained_solution(operator_number,
                                                       _current_solution.score_wvcp());
            _adaptive_helper->update_helper();
            fmt::print(
                Parameters::p->output_tbt,
                "{},{},{},{},{},{}\n",
                Parameters::p->elapsed_time(std::chrono::high_resolution_clock::now()),
                _turn,
                _adaptive_helper->to_str_proba(),
                operator_number,
                score_before_ls,
                _current_solution.score_wvcp());

            _adaptive_helper->increment_turn();
        }

        const int score_wvcp = _current_solution.score_wvcp();
        // update
        _current_node->update(score_wvcp);
        // update and print best score
        if (_best_solution.score_wvcp() > score_wvcp) {
            _t_best = std::chrono::high_resolution_clock::now();
            _best_solution = _current_solution;
            if (Solution::best_score_wvcp > score_wvcp)
                Solution::best_score_wvcp = score_wvcp;
            fmt::print(Parameters::p->output, "{}", line_csv());
            _current_node = nullptr;
            _root_node->clean_graph(_best_solution.score_wvcp());
        }
        _current_node = nullptr;
    }
    _current_node = _root_node;
    fmt::print(Parameters::p->output, "{}", line_csv());
    _current_node = nullptr;
}

void MCTS::selection() {
    while (not _current_node->terminal()) {
        double max_score = std::numeric_limits<double>::min();
        std::vector<std::shared_ptr<Node>> next_nodes;
        for (const auto &node : _current_node->children_nodes()) {
            if (node->score_ucb() > max_score) {
                max_score = node->score_ucb();
                next_nodes = {node};
            } else if (node->score_ucb() == max_score) {
                next_nodes.push_back(node);
            }
        }
        _current_node = rd::choice(next_nodes);
        apply_action(_current_solution, _current_node->move());
    }
}

void MCTS::expansion() {
    const Action next_move = _current_node->next_child();
    apply_action(_current_solution, next_move);
    const auto next_possible_actions = next_possible_moves(_current_solution);
    if (not next_possible_actions.empty()) {
        _current_node =
            std::make_shared<Node>(_current_node.get(), next_move, next_possible_actions);
        _current_node->add_child_to_parent(_current_node);
    }
}

[[nodiscard]] const std::string MCTS::header_csv() const {
    return fmt::format("turn,time,depth,nb total node,nb "
                       "current node,height,{}\n",
                       Solution::header_csv);
}

[[nodiscard]] const std::string MCTS::line_csv() const {
    return fmt::format("{},{},{},{},{},{},{}\n",
                       _turn,
                       Parameters::p->elapsed_time(_t_best),
                       _current_node->get_depth(),
                       Node::get_total_nodes(),
                       Node::get_nb_current_nodes(),
                       Node::get_height(),
                       _best_solution.line_csv());
}

std::vector<Action> next_possible_moves(const Solution &solution) {
    std::vector<Action> moves;
    const int next_vertex = solution.first_free_vertex();
    if (next_vertex == Graph::g->nb_vertices) {
        return moves;
    }

    for (const auto color : solution.non_empty_colors()) {
        if (solution.conflicts_colors(color, next_vertex) == 0) {
            const int next_score =
                solution.score_wvcp() + solution.delta_wvcp_score(next_vertex, color);
            if (Solution::best_score_wvcp > next_score) {
                moves.emplace_back(Action{next_vertex, color, next_score});
            }
        }
    }
    const int next_score = solution.score_wvcp() + Graph::g->weights[next_vertex];
    if (Solution::best_score_wvcp > next_score) {
        moves.emplace_back(Action{next_vertex, -1, next_score});
    }
    std::sort(moves.begin(), moves.end(), compare_actions);
    return moves;
}

void apply_action(Solution &solution, const Action &action) {
    solution.add_to_color(action.vertex, action.color);
    assert(solution.first_free_vertex() == action.vertex);
    solution.increment_first_free_vertex();
    assert(solution.score_wvcp() == action.score);
}

void MCTS::to_dot(const std::string &file_name) const {
    if ((_turn % 5) == 0) {
        std::ofstream file(file_name);
        file << _root_node->to_dot();
        file.close();
    }
}
