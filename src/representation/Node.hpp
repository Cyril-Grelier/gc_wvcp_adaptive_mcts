#pragma once

#include <memory>
#include <vector>

#include "enum_types.hpp"

/**
 * @brief Representation of a node for a MCTS
 *
 */
class Node {

  private:
    /** @brief Number of created nodes*/
    static long total_nodes;
    /** @brief Current number of nodes*/
    static long nb_current_nodes;
    /** @brief Height of the tree*/
    static int height;

    /** @brief Pointer to parent node*/
    Node *_parent_node;
    /** @brief Move that lead from parent node to the current node*/
    Action _move;
    /** @brief Possible next moves to children*/
    std::vector<Action> _possible_moves;
    /** @brief List of children*/
    std::vector<std::shared_ptr<Node>> _children_nodes{};
    /** @brief Number of visits on the node*/
    int _visits;
    /** @brief Score of the node*/
    double _score{};
    /** @brief Score exploration*/
    double _exploration{};
    /** @brief Score exploitation*/
    double _exploitation{};
    /** @brief Score UCB*/
    double _score_ucb{};
    /** @brief Depth of the node*/
    int _depth{};
    /** @brief ID of the node*/
    long _id{};

  public:
    /**
     * @brief Construct a new Node
     *
     * @param parent_node parent node of the new node
     * @param move Action that lead to the new node
     * @param possible_moves possible actions that create new children to the node
     */
    Node(Node *parent_node,
         const Action &move,
         const std::vector<Action> &possible_moves);

    /**
     * @brief Construct a copy of a Node object impossible
     *
     */
    Node(const Node &) = delete;

    /**
     * @brief Destroy the Node object
     *
     */
    ~Node();

    /**
     * @brief Update the branch, recalculate the score and UCB score
     * and increment the number of visits. Delete node that lead to no children
     *
     * @param score new score
     */
    void update(const double &score);

    /**
     * @brief Delete children and possible children if their score is equal or superior to
     * the best found score
     *
     * @param score best found score
     * @return true The child node stay
     * @return false The child node must be deleted
     */
    bool clean_graph(const int &score);

    /**
     * @brief Return next action of the next child of the node
     *
     * @return const Action next child
     */
    [[nodiscard]] const Action next_child();

    /**
     * @brief Add child to its parent children
     *
     * @param child the child to add to its parent
     */
    void add_child_to_parent(const std::shared_ptr<Node> &child);

    /**
     * @brief Return the move that lead to the node
     *
     * @return const Action& the move
     */
    [[nodiscard]] const Action &move() const;

    /**
     * @brief Return the UCB score of the node
     *
     * @return const double& the UCB score
     */
    [[nodiscard]] const double &score_ucb() const;

    /**
     * @brief Return the children of the node
     *
     * @return const std::vector<std::shared_ptr<Node>>& children node
     */
    [[nodiscard]] const std::vector<std::shared_ptr<Node>> &children_nodes();

    /**
     * @brief Get the total number of nodes created
     *
     * @return long number of nodes created
     */
    [[nodiscard]] static long get_total_nodes();

    /**
     * @brief Get the number of current nodes
     *
     * @return long number of existing node
     */
    [[nodiscard]] static long get_nb_current_nodes();

    /**
     * @brief Get the height of the tree
     *
     * @return int height of the tree
     */
    [[nodiscard]] static int get_height();

    /**
     * @brief Get the depth of the node
     *
     * @return int depth of the current node
     */
    [[nodiscard]] int get_depth();

    /**
     * @brief A node is terminal if it has no possible child
     *
     * @return true the node has no child
     * @return false the node has at least one child
     */
    [[nodiscard]] bool terminal() const;

    /**
     * @brief A node is fully explored if it has no child and no possible child
     *
     * @return true the node is fully explored
     * @return false the branch of the node can grow
     */
    [[nodiscard]] bool fully_explored() const;

    /**
     * @brief Get a string representing the tree from the root node
     *
     * @return std::string the tree in string format
     */
    std::string format() const;

    /**
     * @brief Convert Node to dot format
     *
     * @return std::string dot string
     */
    std::string to_dot() const;

    /**
     * @brief Delete equal operator
     *
     * @param node the other node
     * @return Node& the node
     */
    Node &operator=(const Node &node) = delete;

    /**
     * @brief A node is superior if its score is higher
     *
     * @param other the node to compare
     * @return true other s score higher
     * @return false other s score lower or equal
     */
    bool operator<(const Node &other) const;
};
