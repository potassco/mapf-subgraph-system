#pragma once

#include <numeric>
#include <math.h>

#include "../solvers/ASP-INC/clingo.hh"

#include "isolver.hpp"

class AspIncSolver : public ISolver
{
public:
	using ISolver::ISolver;
	void ResetStat(int);
	int Solve(int, int);
	void PrintInstance(int, int);
	int ReadResults(int, int);

private:
	Clingo::Control* ctl = NULL;
};