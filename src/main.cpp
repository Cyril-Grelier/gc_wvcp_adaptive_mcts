#include <csignal>
#include <fstream>
#include <memory>
#include <string>
#include <vector>

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wconversion"
#pragma GCC diagnostic ignored "-Wnull-dereference"
#include "cxxopts.hpp"
#pragma GCC diagnostic pop

#include "methods/LocalSearch.hpp"
#include "methods/MCTS.hpp"
#include "representation/Graph.hpp"
#include "representation/Method.hpp"
#include "representation/Parameters.hpp"
#include "utils/random_generator.hpp"

/**
 * @brief Signal handler to let the algorithm to finish its last turn
 *
 * @param signum signal number
 */
void signal_handler(int signum);

/**
 * @brief parse the argument for the search
 *
 * @param argc : number of arguments given to main
 * @param argv : list of arguments
 * @return std::unique_ptr<Method>
 */
std::unique_ptr<Method> parse(int argc, const char **argv);

int main(int argc, const char *argv[]) {
    // see src/utils/parsing.cpp for default parameters
    // Get the method
    auto method(parse(argc, argv));

    // Set the signal handler to stop the search
    signal(SIGTERM, signal_handler);
    signal(SIGINT, signal_handler);

    // Start the search
    method->run();
    Parameters::p->end_search();
}

void signal_handler(int signum) {
    fmt::print(stderr, "\nInterrupt signal ({}) received.\n", signum);
    Parameters::p->time_stop = std::chrono::high_resolution_clock::now();
}

std::unique_ptr<Method> parse(int argc, const char **argv) {
    // analyse command line options
    try {
        // init cxxopts
        cxxopts::Options options(argv[0], "Program to launch : mcts, local search\n");

        options.positional_help("[optional args]").show_positional_help();

        options.allow_unrecognised_options().add_options()("h,help", "Print usage");

        /****************************************************************************
         *
         *                      Set defaults values down here
         *
         ***************************************************************************/

        options.allow_unrecognised_options().add_options()(
            "p,problem",
            "problem (gcp, wvcp)",
            cxxopts::value<std::string>()->default_value(
                //
                "wvcp"
                // "gcp"
                //
                ));

        options.allow_unrecognised_options().add_options()(
            "i,instance",
            "name of the instance (located in instance/wvcp_reduced/)",
            cxxopts::value<std::string>()->default_value(
                //
                // "R50_1g"
                // "R50_9gb"
                // "flat1000_76_0"
                // "DSJC125.1g"
                // "GEOM20"
                // "GEOM20b"
                // "p23"
                // "zeroin.i.1"
                // "p06"
                // "r06"
                // "p42"
                // "queen10_10"
                // "le450_25b"
                // "queen12_12"
                // "DSJR500.1"
                "DSJC500.5"
                // "p31"
                // "inithx.i.1"
                // "miles250"
                // "R75_1gb"
                // "wap04a"
                // "GEOM110a"
                // "inithx.i.1"
                // "C2000.5"
                // "R75_5gb"
                // "DSJC125.1gb"
                // "zeroin.i.3"
                // "DSJC500.5"
                // "DSJC250.5"
                // "DSJC500.9"
                // "wap01a"
                // "C2000.9"
                // "DSJC250.5"
                // "r1000.5"
                // "le450_25a"
                // "le450_25c"
                // "zeroin.i.3"
                // "mulsol.i.5"
                //
                ));

        options.allow_unrecognised_options().add_options()(
            "m,method",
            "method (mcts, local_search)",
            cxxopts::value<std::string>()->default_value(
                //
                // "mcts"
                "local_search"
                //
                ));

        options.allow_unrecognised_options().add_options()(
            "r,rand_seed",
            "random seed",
            cxxopts::value<int>()->default_value(
                //
                // "1"
                std::to_string(time(nullptr))
                // "3"
                //
                ));

        options.allow_unrecognised_options().add_options()(
            "T,target",
            "if the target score is reach, the search is stopped",
            cxxopts::value<int>()->default_value(
                //
                // "-1"
                "47"
                // "8"
                //
                ));

        options.allow_unrecognised_options().add_options()(
            "u,use_target",
            "for the mcts, if true, the mcts will prune the tree according to the target "
            "otherwise it will use the best found score during the search",
            cxxopts::value<std::string>()->default_value(
                //
                "false"
                // "true"
                //
                ));

        options.allow_unrecognised_options().add_options()(
            "b,objective",
            "for mcts, does the algorithm stop when the target is reached or when "
            "optimality is proven? The algorithm can also stop when the time limit is "
            "reached (optimality, reached)",
            cxxopts::value<std::string>()->default_value(
                //
                // "optimality"
                "reached"
                //
                ));

        const std::string time_limit_default = "3600";
        // const std::string time_limit_default = "18000";
        options.allow_unrecognised_options().add_options()(
            "t,time_limit",
            "maximum execution time in seconds",
            cxxopts::value<int>()->default_value(time_limit_default));

        options.allow_unrecognised_options().add_options()(
            "n,nb_max_iterations",
            "number of iteration maximum for the mcts",
            cxxopts::value<long>()->default_value(
                std::to_string(std::numeric_limits<long>::max())));

        options.allow_unrecognised_options().add_options()(
            "I,initialization",
            "Initialization of the solutions (random, constrained, deterministic, "
            "dsatur, rlf)",
            cxxopts::value<std::string>()->default_value(
                //
                "total_random"
                // "random"
                // "constrained"
                // "deterministic"
                // "dsatur"
                // "rlf"
                //
                ));

        options.allow_unrecognised_options().add_options()(
            "N,nb_iter_local_search",
            "Number max of iteration for local search when call from another method or "
            "not, can be override by max_time_local_search or o and t time",
            cxxopts::value<long>()->default_value(
                //
                std::to_string(std::numeric_limits<long>::max())
                // "500"
                //
                ));

        options.allow_unrecognised_options().add_options()(
            "M,max_time_local_search",
            "Time limit in seconds for local search when call from another method or not "
            "can by override by nb_iter_local_search or o and t time",
            cxxopts::value<int>()->default_value(
                //
                time_limit_default
                // "2"
                // "-1"
                //
                ));

        options.allow_unrecognised_options().add_options()(
            "k,bound_nb_colors",
            "bound on the number of colors, if -1 the bound is the maximum degree of the "
            "graph",
            cxxopts::value<int>()->default_value(
                //
                "-1"
                // "14"
                //
                ));

        options.allow_unrecognised_options().add_options()(
            "c,coeff_exploi_explo",
            "Coefficient exploration vs exploitation for MCTS or for the UCB adaptive "
            "criteria",
            cxxopts::value<double>()->default_value("1")); // 0.25

        options.allow_unrecognised_options().add_options()(
            "A,adaptive",
            "Adaptive selection of operators for algo mem",
            cxxopts::value<std::string>()->default_value(
                //
                "none"
                // "iterated"
                // "random"
                // "deleter"
                // "roulette_wheel"
                // "pursuit"
                // "ucb"
                // "neural_net"
                //
                ));

        options.allow_unrecognised_options().add_options()(
            "w,window_size",
            "Size of the sliding window for adaptive criteria "
            "(for the ones that use it) ",
            cxxopts::value<int>()->default_value("50"));

        options.allow_unrecognised_options().add_options()(
            "l,local_search",
            "Local search selected (to give multiple separate with :)",
            cxxopts::value<std::string>()->default_value(
                //
                // "none"
                // "hill_climbing"
                // "tabu_weight"
                // "tabu_col"
                "random_walk_wvcp"
                // "random_walk_gcp"
                // "partial_col"
                // "afisa"
                // "afisa_original"
                // "redls"
                // "tabu_col_neighborhood"
                // "tabu_weight_neighborhood"
                // "redls_freeze"
                // "ilsts"
                // "ilsts:redls"
                // "ilsts:redls:afisa:tabu_weight:tabu_col"
                // "ilsts:redls:afisa:tabu_weight"
                // "ilsts:redls:afisa:tabu_weight:useless_ls"
                // "ilsts:redls:tabu_weight"
                // "ilsts:useless_ls:redls"
                // "ilsts:tabu_weight:useless_ls"
                // "ilsts:redls:useless_ls"
                //
                ));

        options.allow_unrecognised_options().add_options()(
            "s,simulation",
            "Simulation for MCTS (no_ls, always_ls, depth, fit, depth_fit)",
            cxxopts::value<std::string>()->default_value(
                //
                "no_ls"
                // "always_ls"
                // "fit"
                // "depth"
                // "level"
                // "depth_fit"
                //
                ));

        options.allow_unrecognised_options().add_options()(
            "O,O_time",
            "O to calculate the time of RL : O+P*nb_vertices seconds",
            cxxopts::value<int>()->default_value("0"));

        options.allow_unrecognised_options().add_options()(
            "P,P_time",
            "P to calculate the time of RL : O+P*nb_vertices seconds",
            cxxopts::value<double>()->default_value(
                //
                // "0.2"
                "0.02"
                //
                ));

        options.allow_unrecognised_options().add_options()(
            "o,output_directory",
            "output file, let empty if output to stdout, else directory, file name will "
            "be [instance name]_[rand seed].csv (.running if not finished) add a tbt "
            "repertory for the turn by turn informations",
            cxxopts::value<std::string>()->default_value(""));

        /****************************************************************************
         *
         *                      Set defaults values up here
         *
         ***************************************************************************/

        const auto result = options.parse(argc, const_cast<char **&>(argv));

        // help message
        if (result.count("help")) {
            // load instance names
            std::ifstream i_file("../instances/instance_list_wvcp.txt");
            if (!i_file) {
                fmt::print(stderr,
                           "Unable to find : ../instances/instance_list_wvcp.txt\n"
                           "Check if you imported the submodule instance, commands :\n"
                           "\tgit submodule init\n"
                           "\tgit submodule update\n");
                exit(1);
            }
            std::string tmp;
            std::vector<std::string> instance_names;
            while (!i_file.eof()) {
                i_file >> tmp;
                instance_names.push_back(tmp);
            }
            i_file.close();
            // print help
            fmt::print(stdout,
                       "{}\nInstances :\n{}\n",
                       options.help(),
                       fmt::join(instance_names, " "));
            exit(0);
        }

        // get parameters
        const std::string problem = result["problem"].as<std::string>();
        if (problem != "wvcp" and problem != "gcp") {
            fmt::print(stderr,
                       "unknown problem {}\n"
                       "select :\n"
                       "\twvcp (Weighted Vertex Coloring Problem)\n"
                       "\tgcp (Graph Coloring Problem)",
                       problem);
            exit(1);
        }

        const std::string instance = result["instance"].as<std::string>();
        Graph::init_graph(instance, problem);

        const std::string method = result["method"].as<std::string>();

        const int rand_seed = result["rand_seed"].as<int>();
        rd::generator.seed(rand_seed);

        const int target = result["target"].as<int>();
        const bool use_target = result["use_target"].as<std::string>() == "true";
        const std::string objective = result["objective"].as<std::string>();
        if (objective != "optimality" and objective != "reached") {
            fmt::print(stderr,
                       "unknown objective {}\n"
                       "select :\n"
                       "\toptimality (for MCTS, stop when tree completely explored)\n"
                       "\treached (for MCTS, stop when target reached)",
                       problem);
            exit(1);
        }

        const int time_limit = result["time_limit"].as<int>();
        const long nb_max_iterations = result["nb_max_iterations"].as<long>();
        const std::string initialization = result["initialization"].as<std::string>();
        const long nb_iter_local_search = result["nb_iter_local_search"].as<long>();

        int max_time_local_search = result["max_time_local_search"].as<int>();

        int bound_nb_colors = result["bound_nb_colors"].as<int>();
        // if bound_nb_colors is -1, set it to the max degree + 1
        if (bound_nb_colors == -1) {
            bound_nb_colors =
                (*std::max_element(Graph::g->degrees.begin(), Graph::g->degrees.end())) +
                1;
        }

        const std::string local_search = result["local_search"].as<std::string>();
        const std::string adaptive = result["adaptive"].as<std::string>();
        const int window_size = result["window_size"].as<int>();

        const double coeff_exploi_explo = result["coeff_exploi_explo"].as<double>();
        const std::string simulation = result["simulation"].as<std::string>();

        const int O_time = result["O_time"].as<int>();
        const double P_time = result["P_time"].as<double>();
        if (max_time_local_search == -1) {
            max_time_local_search = std::max(
                1,
                static_cast<int>(static_cast<double>(Graph::g->nb_vertices) * P_time) +
                    O_time);
        }

        const std::string output_directory = result["output_directory"].as<std::string>();

        // init parameters
        Parameters::p = std::make_unique<Parameters>(problem,
                                                     instance,
                                                     method,
                                                     rand_seed,
                                                     target,
                                                     use_target,
                                                     objective,
                                                     time_limit,
                                                     nb_max_iterations,
                                                     initialization,
                                                     nb_iter_local_search,
                                                     max_time_local_search,
                                                     bound_nb_colors,
                                                     local_search,
                                                     adaptive,
                                                     window_size,
                                                     coeff_exploi_explo,
                                                     simulation,
                                                     O_time,
                                                     P_time,
                                                     output_directory);

        // the method can't be created before the parameters
        if (method == "local_search") {
            return std::make_unique<LocalSearch>();
        }
        if (method == "mcts") {
            return std::make_unique<MCTS>();
        }

        fmt::print(stderr,
                   "error unknown method : {}\n"
                   "Possible method : mcts, local_search",
                   method);
        exit(1);

    } catch (const cxxopts::OptionException &e) {
        fmt::print(stderr, "error parsing options: {} \n", e.what());
        exit(1);
    }
}
