#!/bin/bash

# parameters : $1 line number, $2 to_eval file

# get the command line
line=$(head -n "$1" "$2" | tail -n 1)

# get the output directory and file
direc=$(echo "$line" | rev | cut -d" " -f1 | rev)
file=$(python scripts/get_file_name.py "$line")

cd build_release || exit

# eval the command line
if eval "$line"; then
    # once the job is done, check the solution
    cd ../scripts || exit

    # check the solution
    ./solution_checker.sh "$direc"/"$file".csv
else
    echo "error : $line"
    mv "$direc"/"$file".csv.running "$direc"/"$file".csv.error
fi
