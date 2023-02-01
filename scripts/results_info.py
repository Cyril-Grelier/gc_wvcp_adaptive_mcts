import sys


file = sys.argv[1]

with open(file) as f:
    lines = f.readlines()

# 2 firsts lines are the parameters of the search
parameters_header = lines[0][:-1].split(",")
parameters_line = lines[1][:-1].split(",")
parameters = {p_h: p_l for p_h, p_l in zip(parameters_header, parameters_line)}

# 3rd line is the csv header
scores_header = lines[2][:-1].split(",")
# last line is the time at the end of the search, the line before is the last report of the program
scores_last_line = lines[-2][:-1].split(",")
scores = {s_h: s_l for s_h, s_l in zip(scores_header, scores_last_line)}

print(
    parameters["instance"], parameters["problem"], scores["score"], scores["solution"]
)
