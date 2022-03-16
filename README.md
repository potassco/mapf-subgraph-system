# Multi-agent Pathfinding on Large Maps Using Graph Pruning: This Way or That Way?

This is an implementation of graph pruning strategies and shortest paths choices for reduction-based solvers usable on large maps. Specifically, our framework uses the ASP-solver [clingo](https://potassco.org/clingo/).

## Contents

- `./encodings/{asp,sat}` contains the encodings of the ASP-based and SAT-based solver, resp.
- `./resources/instances` contains the scenario and map files used in the paper.
- `./src` contains the source codes in C++ for the strategy framework.
- `./statistics/results.xlsx` contains the measured results used in the paper.
- `./experiment.sh` a script that solves all of the included instances using all of the possible combinations of strategies and underlying solvers.
- `./makefile` a makefile provided for easy compilation and experiment execution.

## Requirements

### Framework System

Building the framework system requires

- GNU make
- `g++` compatible with standard c++11 support

The system is intended to run in a POSIX terminal with GNU [bash](https://www.gnu.org/software/bash/).

### SAT encoding

The SAT encoding requires [Picat](http://picat-lang.org/) binary which is included.

### ASP encoding

The ASP encoding requires ASP system [clingo](https://potassco.org/clingo/) in version 5.5.0 or higher

## Installation

Clone this repository recursively via `git clone --recursive`.

To build the framework system, run `make` in the root directory of your locally cloned repository.

## Usage

To solve a single instance, call the framework via

``` bash
./subgraph_framework  [-h] [-d] -b base_algorithm -i agents_file -s strategy [-t timeout] [-p shortest_path]
	-h                  : prints help and exits
	-d                  : debug print - keep all of the used instance and output files
	-b base_algorithm   : base algorithm to be used. Available options are sat|asp|asp-teg
	-i agents_file      : path to an agents file
	-s strategy         : strategy to be used. Available options are b|m|p|c
	-t timeout          : timeout of the computation. Default value is 300s
	-p shortest_path    : what shortest path to use to create the pruned graph. Available options are single|all|random|diverse. Default is single.
```

To solve all of the included scenario files, call `make experiment`. The result files are stored in the `./statistics` folder.

### Standalone Usage of the ASP Encoding

An optional standalone usage of the ASP encoding (independent of the `subgraph_framework` binary) is explained [here](encodings/asp/README.md).

## References

