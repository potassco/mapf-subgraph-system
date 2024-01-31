# mapf-optimal-solver
Solve a given mapf instance for a given delta where delta is the different between the considered bound and the minimal bound.

For makespan, the minimal bound is the maximum shortest path length of the agents. All agents will be able to move within this makespan

For sum-of-cost, the minimal bound is the sum of the shortest path length of all agents. Agents will have a horizon based on their shortest path length and the delta.

## Instance format

The instances must have facts of the following form:
- ```vertex(V).```
- ```edge(V1, V2).```
- ```agent(A).```
- ```start(A, V).```
- ```goal(A, V).```

where `V` is a vertex, `A` is an agent and `V1` and `V2` are vertices. The delta is passed as a command line argument.

## Requirements
clingo
[cmapf](https://github.com/rkaminsk/cmapf)

## Usage
```
python mapf.py -i <instance> -m <mode> -d <delta>
```
where `<instance>` is the path to the instance file, `<mode>` is either `makespan`, `soc-jump`, `soc-jump-original` or `soc-iter`, and `<delta>` is the delta to consider.