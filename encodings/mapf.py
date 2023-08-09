
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

import clingo

SAT = "SATISFIABLE"
UNSAT = "UNSATISFIABLE"

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
    
    def prepare_ctl(self, files):
        ctl = clingo.Control(self.args)
        for _f in files:
            ctl.load(_f)
        
        return ctl

    def call_clingo(self, cmd):
        try:
            output = subprocess.check_output(cmd)
        except subprocess.CalledProcessError as e:
            output = e.output.decode("utf-8")

        return output

    def on_model(self, model):
        self.model = model.symbols(shown=True)

        #for atom in model.symbols(shown=True):
        #        print(atom)

        #print("\n\n")

    def get_stats(self, ctl):
        #populate stats class
        #print(ctl.statistics)
        Count.add("Choices", ctl.statistics["solving"]["solvers"]["choices"])
        Count.add("Conflicts", ctl.statistics["solving"]["solvers"]["conflicts"])
        Count.add("Time", ctl.statistics["summary"]["times"]["total"])
        Count.add("Solving", ctl.statistics["summary"]["times"]["solve"])
        Count.add("Grounding", ctl.statistics["summary"]["times"]["total"] - ctl.statistics["summary"]["times"]["solve"])

        Count.add("Rules", ctl.statistics["problem"]["lp"]["rules"])
        Count.add("Atoms", ctl.statistics["problem"]["lp"]["atoms"])

        Count.add("Variables", ctl.statistics["problem"]["generator"]["vars"])
        Count.add("Constraints", ctl.statistics["problem"]["generator"]["constraints"])


    def call_clingo_ctl(self, files, args, prg=""):
        ctl = clingo.Control(args + ["--single-shot"])

        ctl.add(prg)

        for _f in files:
            ctl.load(_f)
        
        ctl.ground()
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

    def main_bound(self, instance, encodings, clingo_args=[]):
        

        res = self.call_clingo_ctl(encodings+[instance], clingo_args)
        #res = self.call_clingo_ctl(encodings+instances, clingo_args, extra_atoms)

        self.print_stats(res)


    def main_jump(self, instance, encodings, clingo_args=[]):


        res = self.call_clingo_ctl(encodings+[instance], clingo_args)
        if res == UNSAT:
             self.print_stats(res)
             return


        agent_horizons = {}
        agent_sps = {}
        for atom in self.model:
            if atom.match("soc_lb",1):
                minsoc = atom.arguments[0].number

            elif atom.match("delta",1):
                delta = atom.arguments[0].number

            elif atom.match("makespan",2):
                ag = atom.arguments[0].number
                hor = atom.arguments[1].number
                agent_horizons[ag] = hor
        
            elif atom.match("agent_SP",2):
                ag = atom.arguments[0].number
                hor = atom.arguments[1].number
                agent_sps[ag] = hor

            elif atom.match("cost",1):
                self._bound = atom.arguments[0].number

        #print(f"SoC is {self._bound}")
        #print("Sum of SPs is", minsoc)

        if self._bound == minsoc:
             res = SAT
        elif  self._bound == minsoc + delta:
             res = SAT
        else:
            min_hor = 1000000000000
            for ag, hor in agent_sps.items():
                 min_hor = min(min_hor, hor)

            new_delta = self._bound - minsoc
            new_hor = min_hor + new_delta
            #print(f"New delta is {new_delta}")
            #print(f"New makespan is {new_hor}")
            extra_atoms = f"makespan({new_hor}). delta({new_delta})."
            if len(agent_horizons) != 0:
                 # increase agent makespans by difference between new and old delta
                 for agent, hor in agent_horizons.items():
                    extra_atoms += f"makespan({agent},{hor + new_delta - delta})."

            if len(agent_sps) != 0:
                 # increase agent makespans by difference between new and old delta
                 for agent, hor in agent_sps.items():
                    extra_atoms += f"agent_SP({agent},{hor})."

            res = self.call_clingo_ctl(encodings+[instance], clingo_args+["-c", "jump=1"], extra_atoms)

        for atom in self.model:
            if atom.match("cost",1):
                self._bound = atom.arguments[0].number

        #print(f"new SoC is {self._bound}")
        self.print_stats(res)

if __name__ == "__main__":

    parser = argparse.ArgumentParser()

    parser.add_argument("-i", help="Instance file(s)")
    parser.add_argument("-m", help="Mode of solving", choices=["soc-jump", "soc-iter", "makespan"])
    parser.add_argument("-t", help="Timeout", type=int, default=604800)
  

    args, rest = parser.parse_known_args()

    app = SocMAPF()
    # rest is supposed to be clingo options
    if args.m == "soc-jump":
        process = Process(target=app.main_jump, args=(args.i, ["encodings/solver.lp", "encodings/soc-minimize.lp"], ["--opt-strategy=usc"]))
    elif args.m == "soc-iter":
        process = Process(target=app.main_bound, args=(args.i, ["encodings/solver.lp", "encodings/soc-bound.lp"]))
    elif args.m == "makespan":
        process = Process(target=app.main_bound, args=(args.i, ["encodings/solver.lp"]))
    else:
         print(f"Invalid argument for -m: {args.m}")

    process.start()
    process.join(timeout=args.t)
    if process.is_alive():
        process.terminate()
        print("Timeout: No solution found in the given time")
