#pragma once

#include <numeric>
#include <math.h>

#include "../solvers/ASP/clingo.hh"

#include "isolver.hpp"

class AspSolver : public ISolver
{
public:
	using ISolver::ISolver;
	int Solve(int, int);

private:
	void PrintInstance(int, int);
	int ReadResults(int, int);

	Clingo::Control* ctl = NULL;
	static void WaitForTerminate(int, Clingo::Control*, bool&);

	bool solved;
};