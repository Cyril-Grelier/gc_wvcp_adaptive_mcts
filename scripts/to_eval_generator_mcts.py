"""
Generate to_eval file which list all run to perform for MCTS

to split :
    split -l 1000 -d to_eval_mcts to_eval_mcts

p,problem
i,instance
m,method
r,rand_seed
T,target
u,use_target
b,objective
t,time_limit
n,nb_max_iterations
I,initialization
N,nb_iter_local_search
M,max_time_local_search
k,bound_nb_colors
c,coeff_exploi_explo
A,adaptive
w,window_size
l,local_search
s,simulation
O,O_time
P,P_time
o,output_directory


"""

import os


def get_target(instance: str, problem: str):
    file = f"instances/best_scores_{problem}.txt"
    with open(file, "r", encoding="utf8") as file:
        for line in file.readlines():
            instance_, score, optimal = line[:-1].split(" ")
            if instance_ != instance:
                continue
            if optimal == "*":
                return int(score)
            return 0
    print(f"instance {instance} not found in instances/best_scores_wvcp.txt")


def get_bounds_nb_colors():
    bounds = {}
    with open("bounds_nb_colors.csv", "r", encoding="utf8") as file:
        for line in file.readlines():
            instance, bound = line[:-1].split(" ")
            bounds[instance] = int(bound)
    return bounds


def get_nb_vertices(instance: str):
    file = "instances/instance_info.txt"
    with open(file, "r", encoding="utf8") as file:
        for line in file.readlines():
            instance_, nb_vertices, _ = line[:-1].split(",")
            if instance_ != instance:
                continue
            return int(nb_vertices)
    print(f"instance {instance} not found in instances/instance_info.txt")


problem = "wvcp"  # gcp wvcp

# Choose the set of instances
instances_set = ("pxx", "pxx")
instances_set = ("rxx", "rxx")
instances_set = ("DIMACS_non_optimal", "dimacs_no")
instances_set = ("DIMACS_optimal", "dimacs_o")
instances_set = ("../instances_coeff", "hard_wvcp_coeff")
instances_set = ("../instances_hard_wvcp", "hard_wvcp")
instances_set = ("instance_list_wvcp", "all")

# i,instance
with open(f"instances/{instances_set[0]}.txt", "r", encoding="utf8") as file:
    instances = [line[:-1] for line in file.readlines()]


method = "mcts"

rand_seeds = list(range(1))

bounds = get_bounds_nb_colors()
use_bounds = False
target = 0
use_target = "false"  # false true
objective = "optimality"  # optimality reached
time_limit = 3600 * 1
nb_max_iterations = 9000000000000000000
initializations = [
    # "random",
    # "constrained",
    "deterministic",
]
nb_iter_local_search = 9000000000000000000
max_time_local_search = -1
coeff_exploi_explo = [
    # "0",
    # "0.25",
    # "0.5",
    # "0.75",
    "1",
    # "1.25",
    # "1.5",
    # "1.75",
    # "2",
    # "3",
    # "4",
    # "5",
]
adaptives = [
    "none",
    # "iterated",
    # "random",
    # "deleter",
    # "roulette_wheel",
    # "pursuit",
    # "ucb",
    # "neural_net",
]
window_sizes = [
    # 10,
    # 25,
    50,
    # 75,
    # 100,
]
local_searchs = [
    "none",
    # "tabu_col",
    # "hill_climbing",
    # "afisa",
    # "afisa_original",
    # "tabu_weight",
    # "redls",
    # "ilsts",
    # "redls_freeze",
    # "ilsts:redls:tabu_weight:afisa_original:none"
    # "ilsts:redls:tabu_weight:afisa_original"
    # "ilsts:redls:tabu_weight"
]
simulations = [
    "no_ls",
    # "always_ls",
    # "fit",
    # "depth",
    # "level",
    # "depth_fit",
    # "chance",
]
O_time = 0
P_time = 0.02

output_directory = f"/scratch/LERIA/grelier_c/mcts_no_bounds_{instances_set[1]}"
output_directory = f"./mcts_no_bounds_{instances_set[1]}"
# output_directory = f"ad_{instances_set[1]}"

os.mkdir(f"{output_directory}/")
for initialization in initializations:
    for coeff in coeff_exploi_explo:
        for local_search in local_searchs:
            for simulation in simulations:
                for adaptive in adaptives:
                    # for P_time in P_times:
                    os.mkdir(f"{output_directory}/{adaptive}")
                    os.mkdir(f"{output_directory}/{adaptive}/tbt")

with open("to_eval_mcts", "w", encoding="UTF8") as file:
    for initialization in initializations:
        for coeff in coeff_exploi_explo:
            for local_search in local_searchs:
                for adaptive in adaptives:
                    for window_size in window_sizes:
                        for simulation in simulations:
                            for instance in instances:
                                # nb_max_iterations = int(
                                #     3600 / (0.02 * get_nb_vertices(instance))
                                # )
                                target = get_target(instance, problem)
                                bound = -1
                                if use_bounds:
                                    bound = bounds[instance]
                                for rand_seed in rand_seeds:
                                    file.write(
                                        f"./gc_wvcp "
                                        f" --problem {problem}"
                                        f" --instance {instance}"
                                        f" --method {method}"
                                        f" --rand_seed {rand_seed}"
                                        f" --target {target}"
                                        f" --use_target {use_target}"
                                        f" --objective {objective}"
                                        f" --time_limit {time_limit}"
                                        f" --nb_max_iterations {nb_max_iterations}"
                                        f" --initialization {initialization}"
                                        f" --nb_iter_local_search {nb_iter_local_search}"
                                        f" --max_time_local_search {max_time_local_search}"
                                        f" --bound_nb_colors {bound}"
                                        f" --coeff_exploi_explo {coeff}"
                                        f" --adaptive {adaptive}"
                                        f" --window_size {window_size}"
                                        f" --local_search {local_search}"
                                        f" --simulation {simulation}"
                                        f" --O_time {O_time}"
                                        f" --P_time {P_time}"
                                        f" --output_directory {output_directory}/{adaptive}"
                                        "\n"
                                    )
