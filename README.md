# Reduction-Based Solving of Large-scale Multi-Agent Pathfinding With Graph Pruning

This is an implementation of graph pruning strategies for reduction-based solvers comparable to search-based techniques on large maps, supplementary to [[2]](#2).

## Contents

- `./encodings/{asp,sat}` contains the encodings of the ASP-based and SAT-based solver, resp.
- `./resources/instances` contains the scenario and map files used in the paper.
- `./src` contains the source codes in C++ for the strategy framework.
- `./statistics/results.xlsx` contains the measured results used in the paper.
- `./experiment.sh` a script that solves all of the included instances using all of the possible combinations of strategies and underlying solvers.
- `./makefile` a makefile provided for easy compilation and experiment execution.

We do not include the CBS algorithm as it is not our implementation. A modified CBS implemetnation was kindly provided by Dor Atzmon from Ben Gurion University as a binary file. [[1]](#1)

## Requirements

### Framework System

Building the framework system requires

- GNU make
- `g++` compatible with standard c++11 support

The system is intended to run in a POSIX terminal with GNU [bash](https://www.gnu.org/software/bash/).

### SAT encoding

The SAT encoding requires `Picat` binary which is included.

### ASP encoding

The ASP encoding requires ASP system [clingo](https://potassco.org/clingo/) in version 5.5.0 or higher

## Installation

Clone this repository recursively via `git clone --recursive`.

To build the framework system, run `make` in the root directory of the locally cloned repository.

## Usage

To solve a single instance call the framework via

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

### Standalone Usage of the ASP Encoding

An optional standalone usage of the ASP encoding (independent of the `corridor_framework` binary) is explained [here](encodings/asp/README.md).

## References

<a id="1">[1]</a> Dor Atzmon, Roni Stern, Ariel Felner, Glenn Wagner, Roman Barták, and Neng-FaZhou. 2018.  Robust Multi-Agent Path Finding. In Proceedings of the Eleventh International Symposium on Combinatorial Search, SOCS 2018.

<a id="2">[2]</a> Matej Husár, Jiří Švancara, Philipp Obermeier, Roman Barták, and Torsten Schaub. 2022. Reduction-based Solving of Multi-agent Pathfinding on Large Maps Using Graph Pruning. In Proceedings of the 21th International Conference on Autonomous Agents and MultiAgent Systems, AMAAS 2022.
