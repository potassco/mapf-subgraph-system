#pragma once

#include "isolver.hpp"

class SatSolver : public ISolver
{
public:
	using ISolver::ISolver;
	int Solve(int, int);
	void PrintInstance(int, int);
	int ReadResults(int, int);

private:

};