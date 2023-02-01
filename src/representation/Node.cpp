#include "Node.hpp"

#include <algorithm>
#include <cmath>

#include "Parameters.hpp"

long Node::total_nodes = 0;
long Node::nb_current_nodes = 0;
int Node::height = 1;

Node::Node(Node *parent_node,
           const Action &move,
           const std::vector<Action> &possible_moves)
    : _parent_node(parent_node),
      _move(move),
      _possible_moves(possible_moves),
      _visits(0) {
    if (_parent_node) {
        _depth = _parent_node->_depth + 1;
    }
    if (_depth > height) {
        height = _depth;
    }
    _id = total_nodes;
    ++total_nodes;
    ++nb_current_nodes;
}

Node::~Node() {
    _parent_node = nullptr;
    _children_nodes.clear();
    nb_current_nodes--;
}

void Node::update(const double &score) {
    _score = ((_score * _visits) + score) / (_visits + 1);
    ++_visits;
    if (not _children_nodes.empty()) {
        // delete empty children
        auto it = _children_nodes.begin();
        while (it != _children_nodes.end()) {
            if (it->get()->_children_nodes.empty() and
                it->get()->_possible_moves.empty()) {
                it = _children_nodes.erase(it);
            } else {
                ++it;
            }
        }
        std::stable_sort(
            _children_nodes.begin(),
            _children_nodes.end(),
            [](const std::shared_ptr<Node> &n1, const std::shared_ptr<Node> &n2) {
                return n1->_score > n2->_score;
            });
        const double sum_rank =
            static_cast<double>(_children_nodes.size() * (_children_nodes.size() + 1)) /
            2;
        int i = 0;
        for (auto &child : _children_nodes) {
            child->_exploitation = ++i / sum_rank;
            child->_exploration = std::sqrt(2 * std::log(_visits) / child->_visits);
            child->_score_ucb = child->_exploitation +
                                Parameters::p->coeff_exploi_explo * child->_exploration;
        }
    }
    if (_parent_node) {
        _parent_node->update(score);
    }
}

bool Node::clean_graph(const int &score) {
    // delete possible children with possible too high score
    auto it_m = _possible_moves.begin();
    while (it_m != _possible_moves.end()) {
        if (it_m->score >= score) {
            it_m = _possible_moves.erase(it_m);
        } else {
            ++it_m;
        }
    }
    // delete children with a too high score
    auto it = _children_nodes.begin();
    while (it != _children_nodes.end()) {
        if (it->get()->_move.score >= score) {
            it = _children_nodes.erase(it);
        } else {
            if (it->get()->clean_graph(score)) {
                ++it;
            } else {
                it = _children_nodes.erase(it);
            }
        }
    }

    // if the node have no child and no possible child, then delete it
    if (_children_nodes.empty() and _possible_moves.empty()) {
        return false;
    }
    return true;
}

[[nodiscard]] const Action Node::next_child() {
    const Action move = _possible_moves.back();
    _possible_moves.pop_back();
    return move;
}

void Node::add_child_to_parent(const std::shared_ptr<Node> &child) {
    child->_parent_node->_children_nodes.push_back(child);
}

[[nodiscard]] const Action &Node::move() const {
    return _move;
}

[[nodiscard]] const double &Node::score_ucb() const {
    return _score_ucb;
}

[[nodiscard]] const std::vector<std::shared_ptr<Node>> &Node::children_nodes() {
    return _children_nodes;
}

[[nodiscard]] long Node::get_total_nodes() {
    return total_nodes - 1; // minus one for the current node in MCTS
}

[[nodiscard]] long Node::get_nb_current_nodes() {
    return nb_current_nodes - 1; // minus one for the current node in MCTS
}

[[nodiscard]] int Node::get_height() {
    return height;
}

[[nodiscard]] int Node::get_depth() {
    return _depth;
}

[[nodiscard]] bool Node::terminal() const {
    return not _possible_moves.empty();
}

[[nodiscard]] bool Node::fully_explored() const {
    return _possible_moves.empty() and _children_nodes.empty();
}

std::string Node::format() const {
    return fmt::format("Action v {} c {} visits {} score {} UCB {}\n",
                       _move.vertex,
                       _move.color,
                       _visits,
                       _score,
                       _score_ucb);
    // if (not this->terminal()) {
    //     std::string txt;
    //     for (const auto &child : _children_nodes) {
    //         txt += child->format();
    //     }
    //     return txt;
    // }
    // std::string txt;
    // for (int i = 0; i < _move.vertex; ++i) {
    //     txt += "\t";
    // }
    // txt += fmt::format("Action v {} c {} visits {} score {} UCB {}\n",
    //                    _move.vertex,
    //                    _move.color,
    //                    _visits,
    //                    _score,
    //                    _score_ucb);
    // for (const auto &child : _children_nodes) {
    //     txt += child->format();
    // }
    // return txt;
}

std::string Node::to_dot() const {
    std::string txt;
    if (not _parent_node) {
        txt += "digraph G{\n";
    }
    for (const auto &child : _children_nodes) {
        txt += fmt::format("\n\tn{} -> n{} ;", _id, child->_id);
        // txt += fmt::format("\n\tn{} [label=\"n{}-s{:.0f}-v{}\"];",
        //                    child->id,
        //                    child->id,
        //                    child->_score,
        //                    child->_visits);
    }

    for (const auto &child : _children_nodes) {
        txt += child->to_dot();
    }
    if (not _parent_node) {
        txt += "\n}";
    }
    return txt;
}

bool Node::operator<(const Node &other) const {
    return _score < other._score;
}
