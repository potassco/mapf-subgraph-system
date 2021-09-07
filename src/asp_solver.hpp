#pragma once

#include "isolver.hpp"

class AspSolver : public ISolver
{
public:
	using ISolver::ISolver;
	int Solve(int, int);
	void PrintInstance(int, int);
	int ReadResults(int, int);

private:

};