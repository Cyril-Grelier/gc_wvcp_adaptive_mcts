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
with open(f"../instances/{instances_set[0]}.txt", "r", encoding="utf8") as file:
    instances = [line[:-1] for line in file.readlines()]


method = "local_search"

rand_seeds = list(range(20))

target = 0
use_target = "false"  # false true
objective = "reached"  # optimality reached
time_limit = 3600 * 1
initializations = [
    # "random",
    # "constrained",
    # "deterministic",
    "dsatur",
    "rlf",
]
nb_iter_local_search = 9000000000000000000
max_time_local_search = time_limit
local_searchs = [
    "none",
    # "tabu_col",
    # "hill_climbing",
    # "afisa",
    # "afisa_original",
    # "tabu_weight",
    # "redls",
    # "ilsts",
]

output_directory = f"/scratch/LERIA/grelier_c/ls_{instances_set[1]}"
output_directory = f"../greedy_wvcp_{instances_set[1]}"

# os.mkdir(f"{output_directory}/")
# for initialization in initializations:
#     for local_search in local_searchs:
#         os.mkdir(f"{output_directory}/{initialization}")
#         # os.mkdir(f"{output_directory}/{local_search}/tbt")

with open("to_eval_ls", "w", encoding="UTF8") as file:
    for initialization in initializations:
        for local_search in local_searchs:
            for instance in instances:
                target = 0  # get_target(instance, problem)
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
                        f" --initialization {initialization}"
                        f" --nb_iter_local_search {nb_iter_local_search}"
                        f" --max_time_local_search {max_time_local_search}"
                        f" --local_search {local_search}"
                        f" --output_directory {output_directory}/{initialization}"
                        "\n"
                    )
