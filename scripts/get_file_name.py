import sys

line = sys.argv[-1].split(" ")

try:
    instance = line[line.index("--instance") + 1]
    rand_seed = line[line.index("--rand_seed") + 1]
except Exception as e:
    print(e, file=sys.stderr)
    print(sys.argv, file=sys.stderr)
print(instance + "_" + rand_seed)
