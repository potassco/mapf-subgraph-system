#pragma once

#include <numeric>
#include <math.h>

#include "isolver.hpp"

class AspSolver : public ISolver
{
public:
	using ISolver::ISolver;
	int Solve(int, int);

private:
	void PrintInstance(int, int);
	int ReadResults(int, int);
};