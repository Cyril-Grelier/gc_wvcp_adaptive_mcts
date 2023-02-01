#!/bin/bash

# steps to runs all jobs
echo "do not exec this file"
exit

# build project
./scripts/build.sh
# or
srun --partition=SMP-short --exclude=cribbar[041-056] --time=00:10:00 ./scripts/build_release.sh

# generate folders and list of jobs
python3 scripts/to_eval_generator_ls.py
python3 scripts/to_eval_generator_mcts.py

# split the to_eval file if more than 1000 lines
split -l 1000 -d to_eval_ls to_eval_ls
split -l 1000 -d to_eval_mcts to_eval_mcts

# launch each jobs by 1000 and add the job id after launching each command
# edit slurm_METHOD #SBATCH --array=1-1000 (1 et 1000 included)

sbatch scripts/slurm_ls.sh to_eval_ls
sbatch scripts/slurm_mcts.sh to_eval_mcts

# check for problems
find slurm_output/* -not -size 0
find slurm_output/* -not -size 0 -ls -exec cat {} \;

find slurm_output/ -size 0 -delete

find OUTPUT -name "*.csv.running" -ls
find OUTPUT -name "*.csv.error" -ls

# files with more than one line
find slurm_output/*.out -exec awk 'END { if (NR > 1) print FILENAME }' {} \;
