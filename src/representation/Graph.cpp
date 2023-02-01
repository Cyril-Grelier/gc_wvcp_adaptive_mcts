#include "Graph.hpp"

#include <fstream>

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Weffc++"
#include <fmt/printf.h>
#pragma GCC diagnostic pop

std::unique_ptr<const Graph> Graph::g = nullptr;

void Graph::init_graph(const std::string &instance_name, const std::string problem) {
    // load the edges and vertices of the graph
    std::ifstream file;
    file.open("../instances/reduced_" + problem + "/" + instance_name + ".col");

    if (!file) {
        fmt::print(stderr,
                   "Didn't find {} in ../instances/{}_reduced/ or "
                   "../instances/gcp_reduced/ (if problem == gcp)\n"
                   "Did you run \n\n"
                   "git submodule init\n"
                   "git submodule update\n\n"
                   "before executing the program ?(import instances)\n"
                   "Otherwise check that you are in the build "
                   "directory before executing the program\n",
                   instance_name,
                   problem);
        exit(1);
    }
    int nb_vertices = 0;
    int nb_edges = 0;
    int n1 = 0;
    int n2 = 0;
    std::vector<std::pair<int, int>> edges_list;
    std::string first;
    file >> first;
    while (!file.eof()) {
        if (first == "e") {
            file >> n1 >> n2;
            edges_list.emplace_back(--n1, --n2);
        } else if (first == "p") {
            file >> first >> nb_vertices >> nb_edges;
            edges_list.reserve(nb_edges);
        } else {
            getline(file, first);
        }
        file >> first;
    }
    file.close();

    std::vector<int> weights(nb_vertices, 1);

    if (problem == "wvcp") {
        // load the weights of the vertices
        std::ifstream w_file("../instances/reduced_wvcp/" + instance_name + ".col.w");
        if (!w_file) {
            fmt::print(stderr,
                       "Didn't find weights for {} in ../instances/wvcp_reduced/\n",
                       instance_name);
            exit(1);
        }
        size_t i(0);
        while (!w_file.eof()) {
            w_file >> weights[i];
            ++i;
        }
        w_file.close();
    }

    std::vector<std::vector<bool>> adjacency_matrix(
        nb_vertices, std::vector<bool>(nb_vertices, false));
    std::vector<std::vector<int>> neighborhood(nb_vertices, std::vector<int>(0));
    std::vector<int> degrees(nb_vertices, 0);
    // Init adjacency matrix and neighborhood of the vertices
    for (auto p : edges_list) {
        if (not adjacency_matrix[p.first][p.second]) {
            adjacency_matrix[p.first][p.second] = true;
            adjacency_matrix[p.second][p.first] = true;
            neighborhood[p.first].push_back(p.second);
            neighborhood[p.second].push_back(p.first);
            ++nb_edges;
        }
    }
    // Init degrees_ of the vertices
    for (int vertex = 0; vertex < nb_vertices; ++vertex) {
        degrees[vertex] = static_cast<int>(neighborhood[vertex].size());
    }
    // Uncomment to check if the vertices are well sorted
    // for(int vertex(0); vertex < nb_vertices-1; ++vertex){
    //     if(weights[vertex] < weights[vertex + 1] or (
    //         weights[vertex] == weights[vertex + 1] and
    //         degrees[vertex] < degrees[vertex + 1]
    //     )){
    //         fmt::print(stderr,"error v{}w{}d{} before v{}w{}d{}\nVertices must be
    //         sorted\n", vertex, weights[vertex], degrees[vertex],vertex,
    //         weights[vertex], degrees[vertex]);
    //     }
    // }
    Graph::g = std::make_unique<Graph>(instance_name,
                                       nb_vertices,
                                       nb_edges,
                                       edges_list,
                                       adjacency_matrix,
                                       neighborhood,
                                       degrees,
                                       weights);
}

Graph::Graph(const std::string &name_,
             const int &nb_vertices_,
             const int &nb_edges_,
             const std::vector<std::pair<int, int>> &edges_list_,
             const std::vector<std::vector<bool>> &adjacency_matrix_,
             const std::vector<std::vector<int>> &neighborhood_,
             const std::vector<int> &degrees_,
             const std::vector<int> &weights_)
    : name(name_),
      nb_vertices(nb_vertices_),
      nb_edges(nb_edges_),
      edges_list(edges_list_),
      adjacency_matrix(adjacency_matrix_),
      neighborhood(neighborhood_),
      degrees(degrees_),
      weights(weights_) {
}
