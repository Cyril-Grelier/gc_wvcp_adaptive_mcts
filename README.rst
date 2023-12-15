Graph Coloring - Weighted Vertex Coloring Problem - Adaptive Monte Carlo Tree Search
====================================================================================


|GitHub license|

.. |GitHub license| image:: https://img.shields.io/github/license/Cyril-Grelier/gc_wvcp_adaptive_mcts
   :target: https://github.com/Cyril-Grelier/gc_wvcp_adaptive_mcts/blob/master/LICENSE


This project proposes an MCTS algorithm for the weighted vertex coloring problem (WVCP) that selects local search operators during the search with hyperheuristics.

This problem is a variant of the Graph Coloring Problem. Given a weighted graph :math:`G=(V,E)`, the set of vertices :math:`V`, the set of edges :math:`E` and let :math:`W` be the set of weights :math:`w(v)` associated to each vertex :math:`v` in :math:`V`, the WVCP consists in finding a partition of the vertices :math:`V` in into :math:`k` color groups :math:`S=(V_1,...,Vk)` :math:`(1 \leq k \leq |V|)` such that no adjacent vertices belong to the same color group and such that the objective function :math:`f(S) = \sum_{i=1}^{k}\max_{v\in V_i}{w(v)}` is minimized.

This project is coded in C++ for the calculation part and in Python for the data analysis. This work is related to the article :

Monte Carlo Tree Search with Adaptive Simulation: A Case Study on Weighted Vertex Coloring
Cyril Grelier, Olivier Goudet, Jin-Kao Hao
Proceedings of the 23nd European Conference on Evolutionary Computation in Combinatorial Optimization (EvoCOP 2023) 
https://link.springer.com/chapter/10.1007/978-3-031-30035-6_7

Requirements
------------

To compile this project you need :

-  `cmake 3.14+ <https://cmake.org/>`__
-  gcc/g++ 11+
-  `Python 3.9+ <https://www.python.org/>`__ (for the slurm jobs, data analysis and documentation)
-  `PyTorch 1.11.0+ <https://pytorch.org/>`__

Build the project
-----------------

Clone the project

.. code:: bash

    git clone https://github.com/Cyril-Grelier/gc_wvcp_mcts_adaptive

Go to the project directory

.. code:: bash

    cd gc_wvcp_mcts_adaptive

Load the instances

.. code:: bash

    git submodule init
    git submodule update

Create python environment (you can change the python version in ``scripts/build_python.sh``) :

.. code:: bash

    ./scripts/build_python.sh
    source venv/bin/activate

Import PyTorch :

.. code:: bash

    mkdir thirdparty
    cd thirdparty
    wget https://download.pytorch.org/libtorch/cpu/libtorch-shared-with-deps-1.11.0%2Bcpu.zip
    unzip libtorch-shared-with-deps-1.11.0+cpu.zip


Build and compile the project :

.. code:: bash

    ./scripts/build_release.sh

Run the project :

.. code:: bash

    cd build_release
    ./gc_wvcp --help

Note : The project must be run from the build directory as it will look for the instances in the parent directory.

Prepare jobs for slurm
----------------------

*Note : If you use slurm you may want to compile with (adapt to your cluster) :*

.. code:: bash

    srun --partition=SMP-short --exclude=cribbar[041-056] --time=00:10:00 ./scripts/build.sh


Create a folder for slurm output files :

.. code:: bash

    mkdir slurm_output


``scripts/generator_to_eval_ls.py`` (for local search) and ``scripts/generator_to_eval_mcts.py`` (for mcts) will generate a file with one job per line. See the scripts for the parameters of the jobs. You can run the scripts with the command :

.. code:: bash

    python scripts/generator_to_eval_ls.py
    python scripts/generator_to_eval_mcts.py

This will generate a file ``to_eval`` with all the jobs.

If the file is too long for slurm (often more than 1000 lines), split the file :

.. code:: bash

    split -l 1000 -d to_eval to_eval

Edit the slurm array size in ``slurm_METHOD.sh`` with the line `#SBATCH --array=1-1000` and eventually the time or job name or other parameters.

Then you can submit your job to slurm :

.. code:: bash
    
    sbatch scripts/slurm_METHOD.sh to_eval

When a job starts, it creates a file ``output-file-name.csv.running``. At the end of the job, the file is renamed by deleting the ``.running`` at the end of the name. If all your jobs are done but your file still has the ``.running`` then the job crashed (probably not enough time or memory).

When the jobs are done you can check for problems with :

.. code:: bash
    
    # delete the jobs with no problem (once all your jobs are done)
    find output_slurm/name-of-your-job -size 0 -delete
    # show the problem
    find output_slurm/name-of-your-job -ls -exec cat {} \;
    # To list eventual crash
    find output_test_slurm -name "*.csv.running" -ls

At the end of the slurm jobs, the last solution is checked with a python script to ensure there is no trouble with the solution.


Data analysis
-------------

``scripts/xlsx_generator.py`` takes raw data and converts it to xlsx files (in xlsx_files repertory) with colored best scores and p-value calculation.

Make sure to set all required methods, instances, and output names directly in the script before running it.

The outputs directory already contain the raw data from the results used to write my thesis, those results are more recent and are not included in the article.
They are often better than the results in the article due to the correction of some bugs, some optimizations and the use of the -LTO flag during the compilation.

To un-tar the archives in multiple files use the following command :

    cat wvcp_all_mcts_ls.tgz.a* | tar xzf -

The archives are created with this command :

    split -b 45M wvcp_all_mcts_ls.tgz wvcp_all_mcts_ls.tgz.


The raw data are in the following directories :
	- wvcp greedy : ``outputs/wvcp_all_greedy``
	- wvcp mcts+greedy : ``outputs/wvcp_all_mcts_greedy`` (with or without the use of bounds)
	- gcp greedy : ``outputs/gcp_all_greedy`` (the code used for those results is not available in this repository and not public yet)
	- gcp mcts+greedy : ``outputs/gcp_all_mcts_greedy`` (always with the use of bounds, the code used for those results is not available in this repository and not public yet)
	- wvcp local search : ``outputs/wvcp_all_ls``
	- wvcp mcts+ls : ``outputs/wvcp_all_mcts_ls`` (the number after the ls name is the time in seconds spent in the local search during the simulation t = 0.xx * \|V\|)
	- wvcp mcts+hh : ``outputs/wvcp_all_mcts_hh``

The ``xlsx_files`` directory contains the xlsx files generated from the raw data of the results used to write my thesis, those results are more recent and are not included in the article.
They have been generated with the ``scripts/xlsx_generator.py`` script.
Look at the main function of the script to see how to generate the xlsx files.

Acknowledgements
----------------

We would like to thank Dr. Wen Sun for sharing the binary code of their
AFISA algorithm [1] (the AFISA algorithm has been reimplemented from
the article, `afisa_original`), Dr. Yiyuan Wang for sharing the code
of their RedLS algorithm [2] (the RedLS algorithm has been
reimplemented from the article, `redls`) and Pr. Bruno Nogueira for
sharing the code of their ILS-TS algorithm [3] (some parts of the code
has been used and adapted to the implementation of the project,
`ilsts`).

-  [1] Sun, W., Hao, J.-K., Lai, X., Wu, Q., 2018. Adaptive feasible and
   infeasible tabu search for weighted vertex coloring. Information
   Sciences 466, 203–219. https://doi.org/10.1016/j.ins.2018.07.037
-  [2] Wang, Y., Cai, S., Pan, S., Li, X., Yin, M., 2020. Reduction and
   Local Search for Weighted Graph Coloring Problem. AAAI 34, 2433–2441.
   https://doi.org/10.1609/aaai.v34i03.5624
-  [3] Nogueira, B., Tavares, E., Maciel, P., 2021. Iterated local
   search with tabu search for the weighted vertex coloring problem.
   Computers & Operations Research 125, 105087.
   https://doi.org/10.1016/j.cor.2020.105087



Organization
------------

.. code::

    gc_wvcp_adaptive_mcts
    ├── build
    │   └── build directory use ./scripts/build_release.sh to create
    ├── docs
    │   └── docs directory use ./scripts/build_doc.sh to generate docs
    ├── instances
    │   └── instances from https://github.com/Cyril-Grelier/gc_instances
    ├── outputs
    │   └── all_methods_all_instances.tgz : all results
    ├── scripts
    │   ├── build_doc.sh : build doc
    │   ├── build_python.sh : build python
    │   ├── build_release.sh : build c++ code
    │   ├── run_with_slurm.sh : instruction to run with slurm
    │   ├── slurm_ls.sh : slurm script for ls
    │   ├── slurm_mcts.sh : slurm script for mcts
    │   ├── to_eval_generator_ls.py : prepare jobs for ls
    │   ├── to_eval_generator_mcts.py : prepare jobs for mcts
    │   └── xlsx_generator.py : prepare xlsx files of results
    ├── src
    │   ├── main.cpp
    │   ├── methods
    │   │   └── methods to solve the problem
    │   ├── representation
    │   │   └── Solution Graph and other data representation
    │   └── utils
    │       └── random and vector functions
    ├── thirdparty
    │   └── download and extract pytorch here
    ├── venv
    │   └── python environment
    ├── xlsx_files
    │   └── all_results_all.xlsx : all results of studied methods
    ├── CMakeLists.txt
    ├── LICENSE
    ├── README.rst
    ├── requirements.txt
    └── tbt_mcts.ipynb : generate plots of the execution
