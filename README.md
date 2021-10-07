# On Reduction-based Solving of Large-scale Multi-agent Pathfinding Instances Using Subgraphs
This is a supplementary material for paper #29 titled "On Reduction-based Solving of Large-scale Multi-agent Pathfinding Instances Using Subgraphs" submitted to AAMAS 2022.

## Contents

- encodings - Contains the encodings for both ASP-based solver and SAT-based solver.
- resources - Contains the scenario and map files used in the paper.
- src - Contains the source codes in C++ for the strategy framework.
- statistics - Contains the measured results used in the paper.
- experiment.sh - A script that solves all of the included instances using all of the possible combinations of strategies and underlying solvers.
- makefile - A makefile provided for easy compilation and experiment execution.


## Requirements

The system is intended to run under Unix. To build the system `make` is used. The framework is written in C++ compiled by `g++` compatible with standard c++11. The SAT encoding requires `Picat` binary which is included. The ASP encoding requires `Clingo`. For more details on the ASP encoding see the [readme]() in encodings/asp.

## Usage

To build the system call `make`.
To solve a single instance call the built binary:

``` bash
./corridor_framework [-h] -b base_algorithm -i agents_file -s strategy [-t timeout]
        -h                  : prints help and exits
        -d                  : debug print - keep all of the used instance and output files
        -b base_algorithm   : base algorithm to be used. Available options are sat|asp|asp-teg
        -i agents_file      : path to an agents file
        -s strategy         : strategy to be used. Available options are b|m|p|c
        -t timeout          : timeout of the computation. Default value is 300s
```

To solve all of the included scenario files call `make experiment`. The result files are stored in the statistics folder.

