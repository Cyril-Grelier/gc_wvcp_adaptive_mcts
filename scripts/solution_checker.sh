#!/bin/bash

# shellcheck source=../venv/bin/activate
source ../venv/bin/activate

# get final instance, score and solution from given file
read -r instance problem score solution < <(python ../scripts/results_info.py "$1")

cd ../instances || exit

if ! python3 check_solution.py "$instance" "$problem" "$score" "$solution"; then
    echo "Error : $1"
fi
