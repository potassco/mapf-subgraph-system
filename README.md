# On Reduction-based Solving of Large-scale Multi-agent Pathfinding Instances Using Subgraphs

This is a supplementary material for paper #29 titled "On Reduction-based Solving of Large-scale Multi-agent Pathfinding Instances Using Subgraphs" submitted to AAMAS 2022.

## Contents

- `./encodings/{asp,sat}` contains the encodings for both ASP-based solver and SAT-based solver.
- `./resources/instances` contains the scenario and map files used in the paper.
- `./src` contains the source codes in C++ for the strategy framework.
- `./statistics/results.xlsx` contains the measured results used in the paper.
- `./experiment.sh` a script that solves all of the included instances using all of the possible combinations of strategies and underlying solvers.
- `./makefile` a makefile provided for easy compilation and experiment execution.

We do not include the CBS algorithm as it is not our implementation. A modified CBS implemetnation was kindly provided by Dor Atzmon from Ben Gurion University as a binary file. [[1]](#1)

## Requirements

The system is intended to run in a POSIX terminal with GNU bash. To build the system `make` is used. The framework is written in C++ compiled by `g++` compatible with standard c++11. The SAT encoding requires `Picat` binary which is included. The ASP encoding requires `clingo`. For more details on the ASP encoding see the [readme](encodings/asp/README.md).

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

## References
<a id="1">[1]</a> Dor Atzmon, Roni Stern, Ariel Felner, Glenn Wagner, Roman Barták, and Neng-FaZhou. 2018.  Robust Multi-Agent Path Finding. InProceedings of the EleventhInternational Symposium on Combinatorial Search, SOCS 2018,
