
import os
import sys
import re
import pathlib
path = pathlib.Path("__file__").parent.resolve()
os.chdir(path)
from collections import defaultdict
from typing import Any, Optional, Callable
import argparse
from multiprocessing import Process, Queue

import subprocess

import cmapf

import clingo

SAT = "SATISFIABLE"
UNSAT = "UNSATISFIABLE"
OBJECTIVE = cmapf.Objective.SUM_OF_COSTS

def get_const(ctl, name, default):
    c = ctl.get_const(name)
    if c is None:
        return default
    else:
        return c.number

class Count:
	counts = defaultdict(lambda: 0)

	def __init__(self, name: str):
		self.name = name

	def __enter__(self) -> "Count":
		Count.counts[self.name] += 1

		return self

	def __exit__(self, *exc_info: Any) -> None:
		pass

	def __call__(self, func) -> Callable:
		@functools.wraps(func)
		def wrapper_count(*args, **kwargs):
			with self:
				return func(*args, **kwargs)

		return wrapper_count

	@classmethod
	def add(cls, name, amt=1) -> None:
		Count.counts[name] += amt


class SocMAPF:
    def __init__(self):
        self.model = None
        self._bound = 0

        self.call = ["clingo"]

        self.cmd_clingo = False

    def on_model(self, model):
        self.model = model.symbols(shown=True)

        self._bound = 0
        for atom in model.symbols(shown=True):
            if atom.match("penalty", 2):
                self._bound += 1
            
            #print(atom)

        #print("\n\n")

    def get_stats(self, ctl):
        #populate stats class
        #print(ctl.statistics)
        Count.add("choices", ctl.statistics["solving"]["solvers"]["choices"])
        Count.add("conflicts", ctl.statistics["solving"]["solvers"]["conflicts"])
        Count.add("time", ctl.statistics["summary"]["times"]["total"])
        Count.add("Solving", ctl.statistics["summary"]["times"]["solve"])
        Count.add("grounding", ctl.statistics["summary"]["times"]["total"] - ctl.statistics["summary"]["times"]["solve"])

        Count.add("rules", ctl.statistics["problem"]["lp"]["rules"])
        Count.add("atoms", ctl.statistics["problem"]["lp"]["atoms"])

        Count.add("vars", ctl.statistics["problem"]["generator"]["vars"])
        Count.add("constraints", ctl.statistics["problem"]["generator"]["constraints"])


    def call_clingo_ctl(self, files, args, prg="", delta=0):
        ctl = clingo.Control(args + ["--single-shot"])

        ctl.add(prg)

        for _f in files:
            ctl.load(_f)
        
        # ground only instance
        ctl.ground()

        mapf_problem = cmapf.Problem(ctl)
        mapf_problem.add_sp_length(ctl)

        parts = [("mapf",()),("optimize",())]

        if OBJECTIVE == cmapf.Objective.SUM_OF_COSTS:
            parts.append(("sum_of_cost",()))

            with ctl.backend() as bck:
                atom = bck.add_atom(clingo.Function("delta", [clingo.Number(delta)]))
                bck.add_rule([atom])

            delta_or_makespan = delta

        elif OBJECTIVE == cmapf.Objective.MAKESPAN:
            parts.append(("makespan",()))

            min_horizon = 0
            for agent_sp in ctl.symbolic_atoms.by_signature("sp_length", 2):
                sp = agent_sp.symbol.arguments[1].number
                min_horizon = max(sp, min_horizon)

            with ctl.backend() as bck:
                atom = bck.add_atom(
                    clingo.Function("mks", [clingo.Number(min_horizon + delta)])
                )
                bck.add_rule([atom])

            delta_or_makespan = min_horizon + delta
        
        solvable = mapf_problem.add_reachable(ctl, OBJECTIVE, delta_or_makespan)
        
        if solvable is None:
            # if an agent can not reach its goal, then make the program UNSAT
            #with ctl.backend() as bck:
            #    bck.add_rule([])
            Count.add("Unreachable goal")
            return UNSAT
        
        # ground the rest
        ctl.ground(parts)

        count = cmapf.count_atoms(ctl.symbolic_atoms, "reach", 3)
        Count.add("reach atoms", count)

        print("Working...")
        res = ctl.solve(on_model=self.on_model)
        self.get_stats(ctl)
        
        if res.satisfiable:
            return SAT
        return UNSAT

    def print_stats(self, res):
        for name, count in sorted(Count.counts.items()):
            #print(f"{name:24}  :   {count}")
            print(name + " {:.2f}".format(count))
            #accu[f"{name:24}"] = count
        print(res)
        print(f"Sum of Cost: {self._bound}")

    def main_bound(self, instance, encodings, clingo_args=[], delta=0):
        
        res = self.call_clingo_ctl(encodings+[instance], clingo_args, delta=delta)
        #res = self.call_clingo_ctl(encodings+instances, clingo_args, extra_atoms)

        self.print_stats(res)

    def main_jump(self, instance, encodings, clingo_args=[], old_method=False, delta=0):


        res = self.call_clingo_ctl(encodings+[instance], clingo_args, delta=delta)
        if res == UNSAT:
             self.print_stats(res)
             return


        agent_sps = {}

        for atom in self.model:
            if atom.match("sp_length",2):
                ag = atom.arguments[0].number
                hor = atom.arguments[1].number
                agent_sps[ag] = hor

        minsoc = sum(agent_sps.values())

        print(f"SoC is {self._bound}")
        print("Sum of SPs is", minsoc)

        if self._bound == minsoc:
             res = SAT
        elif  self._bound == minsoc + delta:
             res = SAT
        else:
            print("jumping")
            new_delta = self._bound - minsoc
            
            # set objective in case old jump method was used
            OBJECTIVE = cmapf.Objective.SUM_OF_COSTS

            res = self.call_clingo_ctl(encodings+[instance], clingo_args, delta=new_delta)

        self.print_stats(res)

if __name__ == "__main__":

    parser = argparse.ArgumentParser()

    parser.add_argument("-i", help="Instance file(s)")
    parser.add_argument("-m", help="Mode of solving", choices=["soc-jump", "soc-iter", "makespan", "soc-jump-original"])
    parser.add_argument("-d", help="The delta to consider", type=int, default=0)
    parser.add_argument("-t", help="Timeout", type=int, default=604800)
  

    args, rest = parser.parse_known_args()

    app = SocMAPF()
    # rest is supposed to be clingo options
    if args.m == "soc-jump":
        OBJECTIVE = cmapf.Objective.SUM_OF_COSTS
        process = Process(target=app.main_jump, args=(args.i, ["solvers/ASP/encodings/solver.lp", "solvers/ASP/encodings/soc-minimize.lp"], ["--opt-strategy=usc"], False, args.d))
    if args.m == "soc-jump-original":
        OBJECTIVE = cmapf.Objective.MAKESPAN
        process = Process(target=app.main_jump, args=(args.i, ["solvers/ASP/encodings/solver.lp", "solvers/ASP/encodings/soc-minimize.lp"], ["--opt-strategy=usc"], True, args.d))
    elif args.m == "soc-iter":
        OBJECTIVE = cmapf.Objective.SUM_OF_COSTS
        process = Process(target=app.main_bound, args=(args.i, ["solvers/ASP/encodings/solver.lp", "solvers/ASP/encodings/soc-bound.lp"], [], args.d))
    elif args.m == "makespan":
        OBJECTIVE = cmapf.Objective.MAKESPAN
        process = Process(target=app.main_bound, args=(args.i, ["solvers/ASP/encodings/solver.lp"], [], args.d))
    else:
         print(f"Invalid argument for -m: {args.m}")

    process.start()
    process.join(timeout=args.t)
    if process.is_alive():
        process.terminate()
        print("Timeout: No solution found in the given time")
